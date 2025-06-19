#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>


#include "greatest/greatest.h"
#include "likely.h"


TEST test_likely(void) {
    int x = 1;
    ASSERT(LIKELY(x));

    if (LIKELY(x)) {
        PASS();
    } else {
        FAIL();
    }
}

TEST test_unlikely(void) {
    int x = 0;

    if (UNLIKELY(x)) {
        FAIL();
    } else {
        PASS();
    }
}

TEST test_combined(void) {
    int x = 1;
    int y = 0;

    if (LIKELY(x) && !UNLIKELY(y)) {
        PASS();
    } else {
        FAIL();
    }
}



SUITE(suite_likely) {
    RUN_TEST(test_likely);
    RUN_TEST(test_unlikely);
    RUN_TEST(test_combined);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite_likely);
    GREATEST_MAIN_END();
}
