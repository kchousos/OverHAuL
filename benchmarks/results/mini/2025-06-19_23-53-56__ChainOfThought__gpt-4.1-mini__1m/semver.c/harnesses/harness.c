#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "semver.h"

#include "semver.c"  // Include implementation to resolve linker errors

// Helper to securely copy and null-terminate a substring from data
static char *dup_substr(const uint8_t *data, size_t len) {
    char *str = (char *)malloc(len + 1);
    if (!str) return NULL;
    memcpy(str, data, len);
    str[len] = '\0';
    return str;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 2) return 0;

    // Find a separator zero byte or split in half if none
    size_t split_pos = 0;
    for (size_t i = 0; i < Size; i++) {
        if (Data[i] == 0) {
            split_pos = i;
            break;
        }
    }
    if (split_pos == 0) split_pos = Size / 2;

    if (split_pos == 0 || split_pos == Size) return 0;

    char *str1 = dup_substr(Data, split_pos);
    char *str2 = dup_substr(Data + split_pos, Size - split_pos);
    if (!str1 || !str2) {
        free(str1);
        free(str2);
        return 0;
    }

    // Clean input strings to remove invalid characters to increase chance parsing succeeds
    semver_clean(str1);
    semver_clean(str2);

    semver_t ver1 = {0};
    semver_t ver2 = {0};

    // Parse versions; ignore if invalid
    int res1 = semver_parse(str1, &ver1);
    int res2 = semver_parse(str2, &ver2);

    if (res1 == 0 && res2 == 0) {
        // Perform all comparisons
        volatile int cmp = semver_compare(ver1, ver2);
        volatile int gt = semver_gt(ver1, ver2);
        volatile int lt = semver_lt(ver1, ver2);
        volatile int eq = semver_eq(ver1, ver2);
        volatile int neq = semver_neq(ver1, ver2);
        volatile int gte = semver_gte(ver1, ver2);
        volatile int lte = semver_lte(ver1, ver2);

        // Test satisfies operators =, >=, <=, <, >, ^, ~
        volatile int sat_eq = semver_satisfies(ver1, ver2, "=");
        volatile int sat_gte = semver_satisfies(ver1, ver2, ">=");
        volatile int sat_lte = semver_satisfies(ver1, ver2, "<=");
        volatile int sat_lt = semver_satisfies(ver1, ver2, "<");
        volatile int sat_gt = semver_satisfies(ver1, ver2, ">");
        volatile int sat_caret = semver_satisfies(ver1, ver2, "^");
        volatile int sat_tilde = semver_satisfies(ver1, ver2, "~");

        // Also test numeric conversion
        volatile int num1 = semver_numeric(&ver1);
        volatile int num2 = semver_numeric(&ver2);

        // Test render
        char render_buf1[512] = {0};
        char render_buf2[512] = {0};
        semver_render(&ver1, render_buf1);
        semver_render(&ver2, render_buf2);

        // Test bump functions
        semver_bump(&ver1);
        semver_bump_minor(&ver1);
        semver_bump_patch(&ver1);
    }

    semver_free(&ver1);
    semver_free(&ver2);

    free(str1);
    free(str2);

    return 0;
}