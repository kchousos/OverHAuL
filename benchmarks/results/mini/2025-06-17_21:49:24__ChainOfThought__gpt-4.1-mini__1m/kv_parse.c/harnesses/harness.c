#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "kv_parse_buffer.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Allocate a mutable buffer and null-terminate it */
    char *input = (char *)malloc(size + 1);
    if (!input)
        return 0;

    /* Copy input and ensure null termination */
    memcpy(input, data, size);
    input[size] = '\0';

    /* Key to search for */
    const char *key = "FUZZKEY";
    char value[256];

    char *line = input;
    size_t line_count = 0;

    while (line != NULL)
    {
        char *value_start = kv_parse_buffer_check_key(line, key);
        if (value_start != NULL)
        {
            (void)kv_parse_buffer_get_value(value_start, value, sizeof(value));
        }
        line = kv_parse_buffer_next_line(line, ++line_count);
    }

    free(input);
    return 0;
}