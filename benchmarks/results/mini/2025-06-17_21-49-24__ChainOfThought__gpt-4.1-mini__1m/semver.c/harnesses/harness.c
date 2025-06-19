#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "semver.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Allocate buffer with one extra byte for null terminator
    char *input = (char *)malloc(size + 1);
    if (!input) return 0; // allocation failure is not testable here
    if (size > 0) {
        memcpy(input, data, size);
    }
    input[size] = '\0'; // ensure null-termination for semver_parse

    semver_t ver = {0};
    int res = semver_parse(input, &ver);
    if (res == 0) {
        // Parsed successfully, free allocated strings
        semver_free(&ver);
    }

    free(input);
    return 0;
}