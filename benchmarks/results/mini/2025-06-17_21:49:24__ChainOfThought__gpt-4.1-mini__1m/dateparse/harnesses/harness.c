#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "dateparse.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }
    // dateparse expects a const char* input and a valid length.
    // It does not require null-terminated strings.
    // We can call directly with data cast as char* and passing size.
    date_t dt;
    int offset = 0;
    // We do not need to check return value; just exercise the parser.
    dateparse((const char*)data, &dt, &offset, (int)size);
    return 0;
}