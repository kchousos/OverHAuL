#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// From provided kv_parse_buffer.h interface
char *kv_parse_buffer_next_line(char *str, size_t line_count);
char *kv_parse_buffer_check_key(char *str, const char *key);
size_t kv_parse_buffer_get_value(char *str, char *value, size_t value_max);
size_t kv_parse_buffer_check_section(char *str, char *section, size_t section_max);

// Fuzz target function
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Defensive: We need at least 1 byte to do any meaningful parsing
    if (size == 0) return 0;

    // Make a zero-terminated copy of input (functions expect null-terminated strings)
    // Allocate an extra byte for '\0'
    char *input = malloc(size + 1);
    if (!input) return 0;
    memcpy(input, data, size);
    input[size] = '\0';

    // Define a fixed key to look for in the buffer.
    // Key chosen small to be plausible in fuzzed data.
    const char *key = "key";

    // Buffer to hold extracted value
    char value[256];

    size_t line = 0;
    // Iterate lines in the input buffer using kv_parse_buffer_next_line
    char *line_ptr = input;
    while (line_ptr != NULL) {
        // Check if current line contains the key
        char *value_ptr = kv_parse_buffer_check_key(line_ptr, key);
        if (value_ptr != NULL) {
            // Extract the value for the key
            volatile size_t val_len = kv_parse_buffer_get_value(value_ptr, value, sizeof(value));
            (void)val_len; // suppress unused variable warning
            // Potentially can do additional checks or touch 'value' content here
        }
        // Move to next line
        line++;
        line_ptr = kv_parse_buffer_next_line(line_ptr, line);
    }

    free(input);
    return 0;
}