/**
 * @file kv_file_get_value.c
 * @brief Simple ANSI C Key-Value Parser.
 *
 * This file contains a parser function that extracts values associated with keys in a formatted
 * key-value file (e.g., "key=value" or "key: value").
 *
 * This code is released to the public domain by Brian Khuu in 2025.
 */

#include <stdio.h>

unsigned int kv_file_get_value(FILE *file, const char *key, char *value, unsigned int value_max)
{
    rewind(file);

    for (char ch = getc(file); ch != EOF; ch = getc(file))
    {
#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
        while (ch == ' ' || ch == '\t')
        {
            ch = getc(file);
        }
#endif

        /* Check For Key */
        for (int i = 0; ch != EOF && key[i] != '\0'; i++, ch = getc(file))
        {
            /* Key Mismatched. Skip Line */
            if (ch != key[i])
            {
                goto skip_line;
            }
        }

#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
        while (ch == ' ' || ch == '\t')
        {
            ch = getc(file);
        }
#endif

        /* Check For Key Value Delimiter */
        if (ch != '=' && ch != ':')
        {
            return 0;
        }
        ch = getc(file);

#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
        while (ch == ' ' || ch == '\t')
        {
            ch = getc(file);
        }
#endif

        /* Copy Value To Buffer */
#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
        int quote = EOF;
        int prev = EOF;
#endif
        for (int i = 0; i < (value_max - 1); ch = getc(file))
        {
            if (ch == EOF || ch == '\r' || ch == '\n')
            {
                /* End Of Line. Return Value */
                value[i] = '\0';
#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
                while (i > 0 && (value[i - 1] == ' ' || value[i - 1] == '\t'))
                {
                    i--;
                    value[i] = '\0';
                }
#endif
                return i;
            }
#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
            else if (quote == EOF && (ch == '\'' || ch == '"'))
            {
                /* Start Of Quoted String */
                quote = ch;
                continue;
            }
            else if (quote != EOF && prev != '\\' && ch == quote)
            {
                /* End Of Quoted String. Return Value */
                value[i] = '\0';
                return i;
            }
            else if (quote != EOF && prev == '\\' && ch == quote)
            {
                /* Escaped Character In Quoted String */
                value[i - 1] = ch == EOF ? '\0' : ch;
                continue;
            }

            prev = ch;
#endif

            value[i++] = ch == EOF ? '\0' : ch;
        }

        /* Value too large for buffer. Don't return a value. */
        value[0] = '\0';
        return 0;

    skip_line:
        /* Search for start of next line */
        while (ch != '\n')
        {
            if (ch == EOF)
            {
                /* End of string. Key was not found. */
                return 0;
            }
            ch = getc(file);
        }
    }

    /* End of string. Key was not found. */
    return 0;
}
