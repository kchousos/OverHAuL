#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration of the function under test
int is_email(const char *str);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Allocate a buffer to hold input data plus null terminator
    char *input = (char *)malloc(size + 1);
    if (!input)
        return 0;

    // Copy data and add null terminator
    memcpy(input, data, size);
    input[size] = '\0';

    // Call the function to be fuzzed
    // This may crash if strrchr returns NULL and code subtracts pointer without check
    (void)is_email(input);

    free(input);
    return 0;
}