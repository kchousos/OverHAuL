#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <bacon.h>
#include "decode.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Defensive: Only proceed if size > 0, but decoder can handle empty input gracefully
    // Just call bacon_decode with data treated as char*, NULL alphabet for default
    char *decoded = bacon_decode((const char *)data, NULL);
    if (decoded) {
        // Use the output in some minimal way to prevent optimization removal
        volatile size_t len = strlen(decoded);
        (void)len;
        free(decoded);
    }
    return 0;
}