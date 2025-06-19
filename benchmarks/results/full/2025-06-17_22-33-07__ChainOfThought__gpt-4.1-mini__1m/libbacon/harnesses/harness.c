#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <bacon.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }

    // Copy input data to a null-terminated buffer
    char *input = (char *)malloc(size + 1);
    if (!input) {
        return 0;
    }
    memcpy(input, data, size);
    input[size] = '\0';

    // Call bacon_decode with default alphabet
    char *decoded = bacon_decode(input, NULL);

    // Free allocated memory
    free(decoded);
    free(input);

    return 0;
}