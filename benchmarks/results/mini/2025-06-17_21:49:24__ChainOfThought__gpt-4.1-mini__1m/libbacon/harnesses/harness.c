#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <bacon.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // The encode function expects a null-terminated string as input.
    // Copy input data into a null-terminated buffer.
    // If size is zero, just return early.
    if (size == 0) return 0;

    // Allocate buffer with extra byte for null terminator.
    char *input = (char*) malloc(size + 1);
    if (!input) return 0;

    // Copy and ensure null termination.
    memcpy(input, data, size);
    input[size] = '\0';

    // Call bacon_encode with default alphabet (NULL)
    char *encoded = bacon_encode(input, NULL);

    if (encoded) {
        // Free returned string
        free(encoded);
    }

    free(input);

    return 0;
}