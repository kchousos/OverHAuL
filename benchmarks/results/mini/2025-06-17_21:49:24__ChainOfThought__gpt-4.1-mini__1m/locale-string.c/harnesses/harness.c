#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "locale-string.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Allocate a null-terminated buffer for the input
    char *input = (char *)malloc(Size + 1);
    if (!input) return 0;

    memcpy(input, Data, Size);
    input[Size] = '\0';

    // Convert UTF-8 to locale encoding
    char *locale_str = utf8_to_locale_alloc(input);
    if (locale_str) {
        free(locale_str);
    }

    // Convert locale encoding to UTF-8
    char *utf8_str = utf8_from_locale_alloc(input);
    if (utf8_str) {
#ifdef _WIN32
        free(utf8_str);
#endif
    }

    free(input);
    return 0;
}