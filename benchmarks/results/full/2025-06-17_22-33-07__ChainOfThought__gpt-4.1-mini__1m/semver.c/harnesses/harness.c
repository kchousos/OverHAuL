#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "semver.h"

// LibFuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // Guard against very large input to avoid excessive allocations in library
    if (Size == 0 || Size > 255)
        return 0;

    // Copy input to buffer and ensure null termination
    char buf[256];
    memcpy(buf, Data, Size);
    buf[Size] = '\0'; // null terminate

    semver_t ver1 = {0};
    semver_t ver2 = {0};

    // Parse first semver string
    int res1 = semver_parse(buf, &ver1);
    if (res1 == 0) {
        // Try numeric conversion and rendering (exercise more code paths)
        int num = semver_numeric(&ver1);

        char render_buf[512] = {0};
        semver_render(&ver1, render_buf);

        // Also fuzz a second version for comparison with a simple variation
        // Create a slightly modified version string by bumping patch
        // We construct a second version struct that differs from ver1
        ver2 = ver1;
        semver_bump_patch(&ver2);

        // Compare versions with various operators
        (void)semver_compare(ver1, ver2);
        (void)semver_eq(ver1, ver2);
        (void)semver_neq(ver1, ver2);
        (void)semver_gt(ver1, ver2);
        (void)semver_lt(ver1, ver2);
        (void)semver_gte(ver1, ver2);
        (void)semver_lte(ver1, ver2);

        // Test satisfies with various operators
        (void)semver_satisfies(ver1, ver2, "=");
        (void)semver_satisfies(ver1, ver2, ">=");
        (void)semver_satisfies(ver1, ver2, "<=");
        (void)semver_satisfies(ver1, ver2, "<");
        (void)semver_satisfies(ver1, ver2, ">");
        (void)semver_satisfies(ver1, ver2, "^");
        (void)semver_satisfies(ver1, ver2, "~");

        // Free allocated strings in semver struct
        semver_free(&ver2);
        semver_free(&ver1);
    }

    return 0;
}