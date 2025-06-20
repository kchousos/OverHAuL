#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "buffer.h"

// Include buffer.c to provide implementations during compilation
#include "buffer.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Create a new buffer with default size
    buffer_t *buf = buffer_new();
    if (!buf) return 0;

    // Make a null-terminated copy of fuzz data
    char *format_str = malloc(size + 1);
    if (!format_str) {
        buffer_free(buf);
        return 0;
    }
    memcpy(format_str, data, size);
    format_str[size] = '\0';

    // Call buffer_appendf with fuzzed data as the format string itself,
    // *without* additional parameters to expose format string bugs.
    buffer_appendf(buf, format_str);

    free(format_str);
    buffer_free(buf);
    return 0;
}