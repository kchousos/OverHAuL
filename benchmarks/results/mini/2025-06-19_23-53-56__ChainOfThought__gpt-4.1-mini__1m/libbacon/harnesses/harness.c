#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bacon.h"

#include "encode.c"
#include "decode.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Allocate buffer to hold input plus null terminator
    char *input = (char *)malloc(size + 1);
    if (input == NULL) {
        return 0;
    }

    // Copy input and null terminate
    memcpy(input, data, size);
    input[size] = '\0';

    // Call decode with default alphabet (NULL)
    char *decoded = bacon_decode(input, NULL);
    if (decoded != NULL) {
        free(decoded);
    }

    // Call encode with default alphabet (NULL)
    char *encoded = bacon_encode(input, NULL);
    if (encoded != NULL) {
        free(encoded);
    }

    free(input);
    return 0;
}