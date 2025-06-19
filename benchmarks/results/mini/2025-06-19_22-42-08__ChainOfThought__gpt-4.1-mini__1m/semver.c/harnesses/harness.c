#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "semver.h"
#include "semver.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;
    size_t max_len = size < 256 ? size : 256;

    // Allocate buffer and copy fuzz input
    char *buffer = (char*)malloc(max_len + 1);
    if (!buffer) return 0;
    memcpy(buffer, data, max_len);
    buffer[max_len] = '\0';

    // Test semver_clean and semver_is_valid on prefixes to find bugs quickly
    for (size_t prefix_len = 1; prefix_len <= max_len; prefix_len++) {
        // Make a copy of prefix
        char *prefix = (char*)malloc(prefix_len + 1);
        if (!prefix) break;
        memcpy(prefix, buffer, prefix_len);
        prefix[prefix_len] = '\0';

        // Clean prefix in-place
        if (semver_clean(prefix) == 0) {
            if (semver_is_valid(prefix)) {
                semver_t ver = {0};
                if (semver_parse(prefix, &ver) == 0) {
                    // Use functions on ver for coverage
                    char render_buf[256] = {0};
                    semver_render(&ver, render_buf);
                    semver_numeric(&ver);
                    semver_bump(&ver);
                    semver_bump_minor(&ver);
                    semver_bump_patch(&ver);

                    // Also try semver_parse_version explicitly on the numeric part
                    semver_t ver_num = {0};
                    semver_parse_version(prefix, &ver_num);

                    semver_free(&ver);
                    semver_free(&ver_num);
                }
            }
        }
        free(prefix);
    }

    // Try pairwise comparisons on substrings at varying splits for better coverage
    // Step size to limit iterations
    size_t step = max_len / 16 + 1;
    for (size_t start1 = 0; start1 < max_len; start1 += step) {
        for (size_t start2 = start1 + 1; start2 < max_len; start2 += step) {
            size_t len1 = start2 - start1;
            size_t len2 = max_len - start2;

            if (len1 == 0 || len2 == 0) continue;
            if (len1 > 64 || len2 > 64) continue; // limit substring size for efficiency

            char *part1 = (char*)malloc(len1 + 1);
            char *part2 = (char*)malloc(len2 + 1);
            if (!part1 || !part2) {
                free(part1);
                free(part2);
                continue;
            }
            memcpy(part1, buffer + start1, len1);
            part1[len1] = '\0';
            memcpy(part2, buffer + start2, len2);
            part2[len2] = '\0';

            // Clean substrings
            if (semver_clean(part1) == 0 && semver_clean(part2) == 0) {
                if (semver_is_valid(part1) && semver_is_valid(part2)) {
                    semver_t ver1 = {0};
                    semver_t ver2 = {0};
                    if (semver_parse(part1, &ver1) == 0 && semver_parse(part2, &ver2) == 0) {
                        // Compare versions thoroughly with multiple functions
                        semver_compare(ver1, ver2);
                        semver_compare_version(ver1, ver2);
                        semver_compare_prerelease(ver1, ver2);

                        const char *ops[] = {"=", ">=", "<=", "<", ">", "^", "~"};
                        for (size_t i = 0; i < sizeof(ops)/sizeof(ops[0]); i++) {
                            semver_satisfies(ver1, ver2, ops[i]);
                        }

                        semver_satisfies_caret(ver1, ver2);
                        semver_satisfies_patch(ver1, ver2);

                        semver_eq(ver1, ver2);
                        semver_neq(ver1, ver2);
                        semver_gt(ver1, ver2);
                        semver_gte(ver1, ver2);
                        semver_lt(ver1, ver2);
                        semver_lte(ver1, ver2);

                        // Numeric representation and rendering
                        semver_numeric(&ver1);
                        semver_numeric(&ver2);

                        char buf1[256] = {0};
                        char buf2[256] = {0};
                        semver_render(&ver1, buf1);
                        semver_render(&ver2, buf2);

                        // Bump versions in sequence
                        semver_bump(&ver1);
                        semver_bump_minor(&ver1);
                        semver_bump_patch(&ver1);
                        semver_bump(&ver2);
                        semver_bump_minor(&ver2);
                        semver_bump_patch(&ver2);

                        semver_free(&ver1);
                        semver_free(&ver2);
                    }
                }
            }
            free(part1);
            free(part2);
        }
    }

    free(buffer);
    return 0;
}