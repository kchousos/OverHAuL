#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

size_t occurrences(const char *needle, const char *haystack);

/*
 * libFuzzer entry point
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Need at least 2 bytes to split into needle and haystack
    if (size < 2) {
        return 0;
    }

    // Split input into two parts
    size_t split = size / 2;

    // Allocate buffers for needle and haystack with null terminators
    // Limit buffer size to avoid excessive memory usage
    size_t max_len = 4096;
    size_t needle_len = (split > max_len - 1) ? max_len - 1 : split;
    size_t haystack_len = (size - split > max_len - 1) ? max_len - 1 : size - split;

    char *needle = (char *)malloc(needle_len + 1);
    char *haystack = (char *)malloc(haystack_len + 1);
    if (!needle || !haystack) {
        free(needle);
        free(haystack);
        return 0;
    }

    memcpy(needle, data, needle_len);
    needle[needle_len] = '\0';

    memcpy(haystack, data + split, haystack_len);
    haystack[haystack_len] = '\0';

    // Call the function under test
    (void)occurrences(needle, haystack);

    free(needle);
    free(haystack);

    return 0;
}