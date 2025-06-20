#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "buffer.c"  // include implementation to avoid linker errors

// libFuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }

    // We require at least 8 bytes for from and to parameters
    // If not enough, just use defaults and append empty string
    ssize_t from = 0;
    ssize_t to = -1;

    size_t data_offset = 0;

    if (size >= 8) {
        // Extract 'from' and 'to' as 32-bit signed integers from input data in little-endian
        from = (int32_t)(data[0]) |
               (int32_t)(data[1] << 8) |
               (int32_t)(data[2] << 16) |
               (int32_t)(data[3] << 24);

        to = (int32_t)(data[4]) |
             (int32_t)(data[5] << 8) |
             (int32_t)(data[6] << 16) |
             (int32_t)(data[7] << 24);

        data_offset = 8;
    }

    // Use the rest of data as the input string to append to buffer.
    // Allocate buffer with buffer_new_with_size to hold data_offset_length bytes 
    size_t append_len = size > data_offset ? size - data_offset : 0;

    // Create an initial buffer with some default size
    buffer_t *buf = buffer_new_with_size(append_len > 0 ? append_len : 16);
    if (!buf) return 0;

    // Prepare a sanitized version of input data removing zero bytes (replacing zero bytes with 'A')
    char *append_str = malloc(append_len + 1);
    if (!append_str) {
        buffer_free(buf);
        return 0;
    }

    for (size_t i = 0; i < append_len; i++) {
        append_str[i] = data[data_offset + i] ? data[data_offset + i] : 'A';
    }
    append_str[append_len] = '\0';

    if (append_len > 0) {
        if (buffer_append(buf, append_str) != 0) {
            free(append_str);
            buffer_free(buf);
            return 0;
        }
    }

    free(append_str);

    // Call buffer_slice with from and to
    buffer_t *slice = buffer_slice(buf, (size_t)(from < 0 ? 0 : from), to);

    if (slice) {
        // Optional: call buffer_trim to exercise more functions
        buffer_trim(slice);
        buffer_free(slice);
    }

    buffer_free(buf);

    return 0;
}