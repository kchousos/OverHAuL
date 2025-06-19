#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "dateparse.h"

// The fuzzing entry point; libFuzzer calls this repeatedly with arbitrary input
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // dateparse expects a null terminated string, plus length param
    // We copy to a local buffer so we can add null terminator safely
    if (Size == 0) {
        return 0;
    }
    // Limit max input size for the buffer to avoid overly large stack alloc
    // but problem statement says do not limit input. So allocate buffer dynamically.
    char *buf = (char*)malloc(Size + 1);
    if (!buf) return 0;
    memcpy(buf, Data, Size);
    buf[Size] = '\0';

    date_t out_date = 0;
    int offset = 0;
    // Call dateparse with string length to allow embedded zeros in input
    // It returns 0 on success, -1 on failure - ignored here
    (void)dateparse(buf, &out_date, &offset, (int)Size);

    free(buf);
    return 0;
}