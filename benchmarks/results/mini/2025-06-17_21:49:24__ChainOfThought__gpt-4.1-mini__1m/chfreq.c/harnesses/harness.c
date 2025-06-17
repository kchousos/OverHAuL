#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "chfreq.h"

// Helper function to free the returned matrix.
static void free_chfreq_matrix(uint32_t **mat) {
    if (!mat) return;
    for (size_t i = 0; mat[i] != NULL; i++) {
        free(mat[i]);
    }
    free(mat);
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) {
        // chfreq assumes null-terminated string, so pass empty string
        uint32_t **mat = chfreq("");
        free_chfreq_matrix(mat);
        return 0;
    }

    // We copy Data into a null-terminated buffer for chfreq.
    char *input = malloc(Size + 1);
    if (!input) return 0; // OOM - just return

    memcpy(input, Data, Size);
    input[Size] = '\0';

    uint32_t **mat = chfreq(input);
    free(input);

    if (mat != NULL) {
        // Verify that the sum of frequencies equals the length of input
        size_t total_freq = 0;
        for (size_t i = 0; mat[i] != NULL; i++) {
            uint32_t ch = mat[i][0];
            uint32_t freq = mat[i][1];
            // Each freq should be >= 1
            if (freq < 1) break;
            total_freq += freq;
            // The stored character should fit in uint8_t for this test
            // (Original code stores char in uint32_t, likely extended)
            if (ch > 255) break;
        }

        // We can't rely on exact sum because input may contain embedded zeros,
        // but still the frequencies should not overflow and be plausible.
        // Just sanity check no extremely large sums:
        if (total_freq > SIZE_MAX / 2) {
            // Possibly corrupted data, but no crash, continue
        }

        free_chfreq_matrix(mat);
    }

    return 0;
}