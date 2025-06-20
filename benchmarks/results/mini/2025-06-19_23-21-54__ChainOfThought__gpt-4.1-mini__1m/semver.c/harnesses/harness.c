#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "semver.h"
#include "semver.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    const size_t max_sub_len = 128;
    const size_t stride = 32;
    static const char valid_chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-+";

    if (size == 0) return 0;

    char *input = malloc(max_sub_len + 1);
    if (!input) return 0;

    for (size_t start = 0; start < size; start += stride) {
        size_t max_len = size - start;
        if (max_len > max_sub_len) max_len = max_sub_len;

        // Random length from 1 to max_len (to increase coverage diversity)
        size_t len = 1 + (start % max_len); // deterministic but varies with start

        memcpy(input, data + start, len);
        input[len] = '\0';

        // Quick filter: skip if input contains invalid chars for semver
        int skip = 0;
        for (size_t i = 0; i < len; i++) {
            char c = input[i];
            if (!strchr(valid_chars, c)) {
                skip = 1;
                break;
            }
        }
        if (skip) continue;

        semver_t ver = {0};
        if (semver_parse(input, &ver) == 0) {
            (void)semver_numeric(&ver);

            char render_buf[512] = {0};
            semver_render(&ver, render_buf);

            semver_t ver_patch = ver;
            semver_t ver_minor = ver;
            semver_t ver_major = ver;

            semver_bump_patch(&ver_patch);
            semver_bump_minor(&ver_minor);
            semver_bump(&ver_major);

            (void)semver_compare(ver, ver_patch);
            (void)semver_compare(ver, ver_minor);
            (void)semver_compare(ver, ver_major);

            (void)semver_compare_version(ver, ver_patch);
            (void)semver_compare_prerelease(ver, ver_patch);

            (void)semver_gt(ver_major, ver_minor);
            (void)semver_gte(ver_major, ver_minor);
            (void)semver_lt(ver_minor, ver_major);
            (void)semver_lte(ver_minor, ver_major);
            (void)semver_eq(ver, ver);
            (void)semver_neq(ver, ver_patch);

            const char *ops[] = {"=", ">=", "<=", "<", ">", "^", "~"};
            for (size_t i = 0; i < sizeof(ops)/sizeof(ops[0]); i++) {
                (void)semver_satisfies(ver, ver_patch, ops[i]);
                (void)semver_satisfies(ver_patch, ver, ops[i]);
            }

            semver_free(&ver);
        }

        // Also fuzz semver_clean occasionally on random substrings outside above loop
        if (start % (stride * 4) == 0) {
            size_t clean_len = max_len;
            if (clean_len > 64) clean_len = 64;
            memcpy(input, data + start, clean_len);
            input[clean_len] = '\0';
            (void)semver_clean(input);
        }
    }

    free(input);
    return 0;
}