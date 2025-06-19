#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "semver.h"

// Include the implementation directly to avoid linker errors
#include "semver.c"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Create a null-terminated buffer for the input string
    char *input = (char *)malloc(Size + 1);
    if (!input) {
        return 0; // out of memory, skip this input
    }
    memcpy(input, Data, Size);
    input[Size] = '\0';

    semver_t ver = {0};

    // Call semver_parse to parse the input string
    int ret = semver_parse(input, &ver);
    if (ret == 0) {
        // On success, free allocated memory inside semver_t
        semver_free(&ver);
    }

    free(input);
    return 0;
}