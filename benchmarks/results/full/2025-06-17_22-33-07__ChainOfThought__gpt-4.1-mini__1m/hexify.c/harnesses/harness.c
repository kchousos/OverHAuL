#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hexify.h"

// The function under test is hexify defined in hexify.c

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Allocate output buffer: for hex string, max size is 2*input_size + 1 for null terminator
    size_t out_size = size * 2 + 1;
    if (out_size == 0) {
        // hexify returns 0 if out_size == 0, so just call with minimal non-zero buffer to allow testing
        out_size = 1;
    }
    char *out = (char *)malloc(out_size);
    if (!out) return 0;

    // Cast away constness because hexify requires non-const unsigned char*
    // but we do not modify input buffer, so safe in practice
    hexify((unsigned char *)data, size, out, out_size);

    free(out);
    return 0;
}