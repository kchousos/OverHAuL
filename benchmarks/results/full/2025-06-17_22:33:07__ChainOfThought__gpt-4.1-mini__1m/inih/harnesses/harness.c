#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "ini.h"

// Handler function called for each parsed name=value pair.
// Return 1 to continue parsing, 0 to stop.
static int dummy_handler(void* user, const char* section, const char* name, const char* value) {
    (void)user;
    (void)section;
    (void)name;
    (void)value;
    return 1;
}

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) {
        return 0;
    }

    // Use fmemopen to create a FILE* from the fuzz input buffer.
    // fmemopen is POSIX; on platforms without it, fallback is complex.
#ifdef __linux__
    FILE* f = fmemopen((void*)data, size, "r");
    if (!f) {
        return 0;
    }
#else
    // On non-posix platforms, fallback: create temp file and write input.
    // This fallback is omitted here for clarity and conciseness.
    return 0;
#endif

    // Call the ini parser on the FILE* stream with dummy_handler.
    ini_parse_file(f, dummy_handler, NULL);

    fclose(f);

    return 0;
}