#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "chfreq.h"

#include "chfreq.c"   // Include the implementation to resolve linker error

#define MAX_INPUT_SIZE 256

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0)
        return 0;

    if (size > MAX_INPUT_SIZE)
        size = MAX_INPUT_SIZE;

    // Allocate buffer with space for null terminator
    char *input = (char *) malloc(size + 1);
    if (!input)
        return 0;

    memcpy(input, data, size);
    input[size] = '\0';

    uint32_t **freq = chfreq(input);
    free(input);

    if (!freq)
        return 0;

    // Verify the array is null-terminated and free elements safely
    size_t i = 0;
    while (freq[i] != NULL) {
        volatile uint32_t ch = freq[i][0];
        volatile uint32_t count = freq[i][1];
        (void)ch;
        (void)count;
        free(freq[i]);
        i++;
        // To avoid infinite loops if not null-terminated correctly
        if (i > MAX_INPUT_SIZE) {
            break;
        }
    }
    free(freq);

    return 0;
}