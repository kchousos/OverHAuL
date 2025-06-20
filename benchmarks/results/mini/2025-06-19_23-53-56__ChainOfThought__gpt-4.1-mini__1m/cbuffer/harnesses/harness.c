#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cbuffer.h"
#include "cbuffer.c"  // Include the implementation to resolve linker errors.

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 1) {
        return 0;
    }

    // Use the first byte to determine the order for creating cbuf
    // Limit order to between 4 (16 bytes) and 12 (4096 bytes) to avoid excessive memory
    unsigned int order = (Data[0] % 9) + 4; // 4 to 12 inclusive

    cbuf_t *cb = cbuf_new(order);
    if (!cb) return 0;

    size_t pos = 1;
    while (pos + 2 <= Size) {
        uint8_t cmd = Data[pos];
        uint8_t len = Data[pos + 1];
        pos += 2;

        if (cmd % 2 == 0) {
            // Offer command: write len bytes from Data if available
            // Cap len to not exceed remaining data
            if (pos + len > Size) {
                len = Size - pos;
            }
            if (len > 0) {
                cbuf_offer(cb, &Data[pos], len);
                pos += len;
            }
        } else {
            // Poll command: poll len bytes from buffer
            if (len > 0) {
                // cbuf_poll is dangerous if len > used space, we deliberately try to poll len anyway
                // but if buffer empty cbuf_poll returns NULL gracefully
                (void)cbuf_poll(cb, len);
            }
        }
    }

    // Cleanup
    cbuf_free(cb);

    return 0;
}