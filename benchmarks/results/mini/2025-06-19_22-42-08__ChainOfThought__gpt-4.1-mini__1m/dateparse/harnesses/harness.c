#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "dateparse.c"
#include "dateparse.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size > 128) return 0;

    // Quick heuristic: skip inputs that do not contain any digits or common date chars
    int found_digit_or_datechar = 0;
    for (size_t i = 0; i < Size; i++) {
        unsigned char c = Data[i];
        if (isdigit(c) || c == '-' || c == '/' || c == ' ' || c == ':' || c == ',' || isalpha(c)) {
            found_digit_or_datechar = 1;
            break;
        }
    }
    if (!found_digit_or_datechar) {
        // Skip input unlikely to parse as a date
        return 0;
    }

    date_t dt = 0;
    int offset = 0;

    dateparse((const char*)Data, &dt, &offset, (int)Size);

    return 0;
}