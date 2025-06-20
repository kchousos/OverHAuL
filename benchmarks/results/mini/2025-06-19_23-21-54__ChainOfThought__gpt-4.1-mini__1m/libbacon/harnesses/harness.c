#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <bacon.h>

// Include implementations directly to resolve linker errors
#include "encode.c"
#include "decode.c"

/*
 * NOTE:
 * To successfully build and link this harness standalone, the encode.c and decode.c 
 * files must be accessible in the same directory.
 * Example build command if not including encode.c and decode.c inside:
 * clang -fsanitize=fuzzer -o harness harness.c encode.c decode.c
 */

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Allocate a buffer that is one byte longer and null terminate for safe strlen usage
    char *input = (char*)malloc(size + 1);
    if (!input) return 0;

    memcpy(input, data, size);
    input[size] = '\0'; // null terminate

    // Test bacon_encode with default alphabet
    char *encoded = bacon_encode(input, NULL);
    if (encoded) {
        // Test bacon_decode on encoded string
        char *decoded = bacon_decode(encoded, NULL);
        if (decoded) {
            free(decoded);
        }
        free(encoded);
    }

    // Also test bacon_decode directly on input (may help expose bugs)
    char *decoded2 = bacon_decode(input, NULL);
    if (decoded2) {
        free(decoded2);
    }

    free(input);

    return 0;
}