#include <stdint.h>
#include <stddef.h>
#include "dateparse.h"

// Include implementation to avoid linker errors
#include "dateparse.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;

    date_t dt = 0;
    int offset = 0;

    // dateparse returns 0 on success, -1 on failure, but we ignore here.
    dateparse((const char*)data, &dt, &offset, (int)size);

    return 0;
}