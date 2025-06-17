#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cbuffer.h"

// libFuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 1) return 0; // Need at least 1 byte for order or operations

    // Fix buffer order to 8 (256 bytes) to avoid large allocations or special mmap failures
    unsigned int order = 8;
    cbuf_t *cb = cbuf_new(order);
    if (!cb) return 0;

    size_t pos = 0;

    while (pos + 1 < size) {
        // Interpret one control byte to decide operation
        uint8_t op = data[pos] % 2; // 0 = offer, 1 = poll
        pos++;

        if (op == 0) {
            // offer operation
            // Next byte is the size of data to offer
            uint8_t offer_size = data[pos];
            pos++;

            // Fix offer size to be at most remaining input or buffer capacity
            size_t max_offer = cb->size;
            size_t can_offer = cbuf_unusedspace(cb);
            size_t to_offer = offer_size;
            if (to_offer > can_offer) to_offer = can_offer;
            if (to_offer > size - pos) to_offer = size - pos;
            if (to_offer > max_offer) to_offer = max_offer;

            if (to_offer > 0) {
                cbuf_offer(cb, &data[pos], (int)to_offer);
            }
            pos += to_offer;
        } else {
            // poll operation
            // Next byte is the size to poll
            uint8_t poll_size = data[pos];
            pos++;

            // This size may exceed used space - here is where potential bug may appear
            if (poll_size > 0) {
                // Call cbuf_poll with poll_size but no check if it's valid
                // May expose memory issues or assertions
                // No usage of returned pointer, just call it
                cbuf_poll(cb, poll_size);
            }
        }
    }

    cbuf_free(cb);
    return 0;
}