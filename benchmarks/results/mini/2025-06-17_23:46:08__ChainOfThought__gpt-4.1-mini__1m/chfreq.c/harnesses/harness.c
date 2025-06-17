#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "chfreq.h"
#include "chfreq.c"  // Include the implementation to resolve linker errors

// Free the matrix returned by chfreq to prevent leak
static void free_chfreq_matrix(uint32_t **mat) {
    if (!mat) return;
    for (size_t i = 0; mat[i] != NULL; i++) {
        free(mat[i]);
    }
    free(mat);
}

// libFuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Allocate a buffer one larger than input to null terminate
    char *input = (char *)malloc(size + 1);
    if (!input) return 0;
    memcpy(input, data, size);
    input[size] = '\0';

    // Call chfreq with this input
    uint32_t **freq = chfreq(input);

    // Optional: iterate over freq matrix to simulate usage
    if (freq) {
        for (size_t i = 0; freq[i] != NULL; i++) {
            // Access character and frequency, but do not print to avoid overhead
            volatile uint32_t ch = freq[i][0];
            volatile uint32_t count = freq[i][1];
            (void)ch;
            (void)count;
        }
        free_chfreq_matrix(freq);
    }

    free(input);
    return 0;
}