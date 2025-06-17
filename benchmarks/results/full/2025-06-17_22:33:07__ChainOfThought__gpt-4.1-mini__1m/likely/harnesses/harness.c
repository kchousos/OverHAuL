#include <stdint.h>
#include <stddef.h>
#include "likely.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // If no input, do nothing
    if (Size == 0) {
        return 0;
    }

    // Convert up to 4 bytes of input into a uint32_t integer
    uint32_t value = 0;
    size_t len = Size < 4 ? Size : 4;
    for (size_t i = 0; i < len; i++) {
        value = (value << 8) | Data[i];
    }

    // Use LIKELY macro with a condition
    if (LIKELY(value == 0x12345678)) {
        // do nothing
    } else if (UNLIKELY(value == 0x87654321)) {
        // do nothing
    } else {
        // else branch
    }

    return 0;
}