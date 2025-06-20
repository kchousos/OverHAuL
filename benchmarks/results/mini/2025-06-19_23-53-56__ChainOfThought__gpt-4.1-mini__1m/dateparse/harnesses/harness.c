#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Include the header of the project to get the prototype of dateparse etc.
#include "dateparse.h"

// Include the implementation directly to resolve linking issues
#include "dateparse.c"

// Required by libFuzzer
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Limit input size for efficient fuzzing and to avoid buffer issues
    const size_t max_size = 60; // increased from 64 to 60 to match internal parseTime limit

    if (size == 0) {
        return 0; // ignore empty inputs
    }
    if (size > max_size) {
        size = max_size;
    }

    // Allocate buffer for input plus null terminator
    char *input = (char *)malloc(size + 1);
    if (!input) {
        return 0; // memory allocation failed, skip
    }

    // Copy input and null terminate
    memcpy(input, data, size);
    input[size] = '\0';

    date_t dt = 0;
    int offset = 0;

    // Call dateparse with input and size
    (void)dateparse(input, &dt, &offset, (int)size);

    free(input);
    return 0;
}