/**
 * @file kv_parse_buffer.h
 * @brief Composible ANSI C Key-Value Parser
 *
 * This file contains a parser function that extracts values associated with keys in a formatted
 * key-value file (e.g., "key=value" or "key: value").
 *
 * Copyright (c) 2025 Brian Khuu
 * MIT licensed
 *
 * @example Usage Example:
 * @code
 * for (size_t line = 0; (input = kv_parse_buffer_next_line(input, line)) != NULL; line++)
 * {
 *     char *input_value = NULL;
 *     if ((input_value = kv_parse_buffer_check_key(input, key)) != NULL)
 *     {
 *         return kv_parse_buffer_get_value(input_value, value, value_max);
 *     }
 * }
 * @endcode
 *
 */
#ifndef KV_PARSE_BUFFER_H
#define KV_PARSE_BUFFER_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Advances the string pointer to the next line in the buffer.
 *
 * This function scans the given string until it finds a newline character ('\n'),
 * then returns a pointer to the start of the next line. If the end of the string is
 * reached, it returns NULL.
 *
 * @param str Pointer to the current position in the string buffer.
 * @param line_count The current line number (0-based). If line_count is 0, the function
 *        returns the input pointer unchanged.
 *
 * @return Pointer to the start of the next line, or NULL if the end of the string is reached.
 */
char *kv_parse_buffer_next_line(char *str, size_t line_count);

/**
 * @brief Checks if the given line contains the specified key.
 *
 * This function compares the start of the given string with the provided key.
 * If the key is found, it returns a pointer to the character following the key-value delimiter ('=' or ':').
 *
 * @param str Pointer to the string buffer containing the key-value pair.
 * @param key The key to search for in the buffer.
 *
 * @return Pointer to the value portion of the string if the key is found, otherwise NULL.
 */
char *kv_parse_buffer_check_key(char *str, const char *key);

/**
 * @brief Extracts the value associated with a key from a string.
 *
 * This function copies the value from the given string into the provided buffer.
 * It stops at the end of the line or when the buffer reaches its maximum size.
 * Optionally, it can handle quoted strings if KV_PARSE_QUOTED_STRINGS is defined.
 *
 * @param str Pointer to the start of the value in the key-value pair.
 * @param value Buffer to store the extracted value.
 * @param value_max Maximum size of the value buffer.
 *
 * @return The length of the extracted value, or 0 if the value is too large or empty.
 */
size_t kv_parse_buffer_get_value(char *str, char *value, size_t value_max);

/**
 * @brief Parses and extracts an INI/TOML-style section header.
 *
 * This function scans a file to detect and extract a section header of the format `[Section]`.
 * The extracted section name (without brackets) is stored in the provided buffer.
 *
 * @param file Pointer to an open file stream.
 * @param section Buffer to store the extracted section name.
 * @param section_max Maximum size of the buffer (including null terminator).
 * @return The length of the extracted section name on success, 0 on failure.
 *
 * @note If the section name exceeds `section_max - 1`, the function resets the file position and returns 0.
 */
size_t kv_parse_buffer_check_section(char *str, char *section, size_t section_max);
#endif
