#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Create a new buffer with default size
    buffer_t *buf = buffer_new();
    if (!buf) return 0;

    // Append fuzz data to buffer - treat as a string, but data may not be zero-terminated
    // Use buffer_append_n to append entire input as bytes
    // This exercises resize and realloc safety
    buffer_append_n(buf, (const char *)data, size);

    // Optionally invoke trimming functions to stress internal state
    if (size > 2) {
        buffer_trim(buf);
    }

    // Also test prepend: prepend a small segment of input or a fixed small string
    if (size > 0) {
        // Use first byte as length for prepend, limit to reasonable size
        size_t prepend_len = data[0] % (size + 1);
        if (prepend_len > 0 && prepend_len <= size) {
            // create temporary null-terminated string
            char *prep_str = (char *)malloc(prepend_len + 1);
            if (prep_str) {
                memcpy(prep_str, data, prepend_len);
                prep_str[prepend_len] = '\0';
                buffer_prepend(buf, prep_str);
                free(prep_str);
            }
        }
    }

    // Free buffer resources
    buffer_free(buf);

    return 0;
}