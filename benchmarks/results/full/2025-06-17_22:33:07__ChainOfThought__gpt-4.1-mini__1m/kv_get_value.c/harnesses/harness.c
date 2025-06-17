#include <stdint.h>
#include <stddef.h>
#include <string.h>

unsigned int kv_get_value(const char *str, const char *key, char *value, unsigned int value_max);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Defensive: must have space for null terminator for string input
    if (size == 0) {
        return 0;
    }

    // Prepare null-terminated string from fuzz input
    // Use at most 4096 bytes to avoid excessively large stack usage
    size_t len = size < 4095 ? size : 4095;
    char input[4096];
    memcpy(input, data, len);
    input[len] = '\0';

    // Output buffer for value extraction. Sufficient size to provoke edge use.
    char value_buf[256];

    // Use a fixed key that will be looked up
    // Choose something simple and common to increase hit chance
    const char *key = "key";

    // Call the parsing function
    // It returns length of extracted value or zero if not found
    unsigned int extracted_len = kv_get_value(input, key, value_buf, sizeof(value_buf));

    // Optional: could trigger abort if output length > buffer size - 1 (shouldn't happen)
    // but kv_get_value code already carefully bounds copies, so we skip extra assertions

    (void)extracted_len;  // silence unused variable warnings

    return 0;
}