#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "chfreq.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Allocate a buffer with null terminator
    char *input = (char *)malloc(Size + 1);
    if (!input) return 0;
    memcpy(input, Data, Size);
    input[Size] = '\0';

    uint32_t **freq = chfreq(input);
    free(input);

    if (freq) {
        // Free the allocated matrix
        int i = 0;
        while (freq[i]) {
            free(freq[i]);
            i++;
        }
        free(freq);
    }

    return 0;
}