#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "buffer.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;

    // Make a null-terminated copy of input for buffer_new_with_copy
    // Adding one byte for null terminator
    char *input_str = malloc(size + 1);
    if (!input_str) return 0;
    memcpy(input_str, data, size);
    input_str[size] = '\0';

    // Create buffer from input
    buffer_t *buf = buffer_new_with_copy(input_str);
    if (!buf) {
        free(input_str);
        return 0;
    }

    // Attempt to append a substring of input if size > 2
    if (size > 2) {
        size_t append_len = (size - 1) / 2;
        // Defensive: limit append_len to avoid excessive sizes
        if (append_len > 1024) append_len = 1024;
        buffer_append_n(buf, input_str + 1, append_len);
    }

    // Attempt to prepend a substring of input if size > 3
    if (size > 3) {
        // Defensive: limit prepend length for safe memmove
        size_t prepend_len = (size - 2) / 2;
        if (prepend_len > 1024) prepend_len = 1024;

        // For prepend, buffer_prepend accepts char*, but input_str is mutable
        buffer_prepend(buf, input_str + 2);
    }

    // Attempt a slice. Use from = 0, to as a signed integer from first byte (or zero)
    ssize_t slice_to = 0;
    if (size > 0) {
        // Interpret first byte as signed value (in range -128..127)
        slice_to = (ssize_t)((int8_t)data[0]);
    }
    buffer_t *slice = buffer_slice(buf, 0, slice_to);
    if (slice) {
        buffer_free(slice);
    }

    // Attempt to trim whitespace
    buffer_trim(buf);

    // Attempt buffer_compact
    buffer_compact(buf);

    // Attempt to fill and clear buffer
    buffer_fill(buf, 'A');
    buffer_clear(buf);

    // Clean up
    buffer_free(buf);
    free(input_str);
    return 0;
}