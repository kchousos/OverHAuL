#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cbuffer.h"

// Include the implementation to resolve linker errors.
#include "cbuffer.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;

    // Use a smaller buffer size to facilitate wrap-around and edge testing.
    cbuf_t* cb = cbuf_new(10); // 1024 byte buffer for faster wrap-around
    if (!cb) return 0;

    size_t pos = 0;
    // Process the input data as a sequence of commands:
    // Each command:
    // - First byte: operation (0 = offer, 1 = poll)
    // - Next byte(s): size for that operation (if possible)
    while (pos + 2 <= size) {
        uint8_t op = data[pos++];
        uint8_t length = data[pos++];
        int len = length % (cbuf_size(cb) / 2 + 1); // limit length to half buffer max +1

        if (op == 0) {
            // Offer data if any data left
            if (pos + len <= size) {
                int written = cbuf_offer(cb, &data[pos], len);
                pos += len > 0 ? len : 0;
                (void)written;
            } else {
                break;
            }
        } else if (op == 1) {
            // Poll up to len bytes if available
            int available = cbuf_usedspace(cb);
            int to_poll = available < len ? available : len;
            if (to_poll > 0) {
                unsigned char* poll_ptr = cbuf_poll(cb, to_poll);
                (void)poll_ptr;
            }
        } else {
            // Unknown operation, skip one byte (to avoid infinite loop)
            pos++;
        }
    }

    // Finally, peek and poll remaining data to test edge states.
    unsigned char* peek_ptr;
    while ((peek_ptr = cbuf_peek(cb)) != NULL) {
        int avail = cbuf_usedspace(cb);
        unsigned char* p = cbuf_poll(cb, avail);
        (void)p;
    }

    cbuf_free(cb);
    return 0;
}