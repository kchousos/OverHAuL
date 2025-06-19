#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "locale-string.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Allocate a null-terminated copy of Data for input string
    char *input = (char*)malloc(Size + 1);
    if (!input) return 0;
    memcpy(input, Data, Size);
    input[Size] = '\0';

    // Call utf8_to_locale_alloc and free result if allocated
    char* converted1 = utf8_to_locale_alloc(input);
    if (converted1) {
#ifdef _WIN32
        free(converted1);
#endif
    }

    // Call utf8_from_locale_alloc and free result if allocated
    char* converted2 = utf8_from_locale_alloc(input);
    if (converted2) {
#ifdef _WIN32
        free(converted2);
#endif
    }

    free(input);
    return 0;
}