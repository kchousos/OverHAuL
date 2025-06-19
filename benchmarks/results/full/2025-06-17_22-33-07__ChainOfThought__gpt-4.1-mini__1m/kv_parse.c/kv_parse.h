/**
 * @file kv_parse.h
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
 * for (size_t line = 0; kv_parse_next_line(file, line); line++)
 * {
 *     if (kv_parse_check_key(file, key))
 *     {
 *         return kv_parse_get_value(file, value, value_max);
 *     }
 * }
 * @endcode
 */
#ifndef KV_PARSE_H
#define KV_PARSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/**
 * @brief Advances the file stream to the next line.
 *
 * This function reads characters from the file until it encounters a newline character ('\n')
 * or reaches the end of the file (EOF). It is useful for skipping lines when parsing.
 *
 * @param file Pointer to the file stream.
 * @param line_count The current line number (0-based). If line_count is 0, the function
 *        does not advance and returns true.
 *
 * @return true if the function successfully moves to the next line, false if EOF is reached.
 */
bool kv_parse_next_line(FILE *file, size_t line_count);

/**
 * @brief Checks if the current line in the file contains the specified key.
 *
 * This function reads characters from the file to match a given key at the beginning of a line.
 * If the key is found and followed by a delimiter ('=' or ':'), the function returns true.
 * Otherwise, it restores the file position to the start of the line.
 *
 * @param file Pointer to the file stream.
 * @param key The key to search for within the file.
 *
 * @return true if the key is found, false otherwise.
 *
 * @note If KV_PARSE_WHITESPACE_SKIP is defined, leading whitespace before the key is ignored.
 */
bool kv_parse_check_key(FILE *file, const char *key);

/**
 * @brief Extracts the value associated with a key from a file stream.
 *
 * This function reads the value after a key-value delimiter ('=' or ':') and stores it
 * in the provided buffer. It stops at the end of the line or when the buffer reaches its maximum size.
 *
 * @param file Pointer to the file stream.
 * @param value Buffer to store the extracted value.
 * @param value_max Maximum size of the value buffer.
 *
 * @return The length of the extracted value, or 0 if the value is too large or empty.
 *
 * @note If KV_PARSE_WHITESPACE_SKIP is defined, leading and trailing whitespace is trimmed.
 * @note If KV_PARSE_QUOTED_STRINGS is defined, quoted values are supported, allowing
 *       values to be enclosed in single or double quotes.
 */
size_t kv_parse_get_value(FILE *file, char *value, size_t value_max);

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
size_t kv_parse_check_section(FILE *file, char *section, size_t section_max);

#endif
