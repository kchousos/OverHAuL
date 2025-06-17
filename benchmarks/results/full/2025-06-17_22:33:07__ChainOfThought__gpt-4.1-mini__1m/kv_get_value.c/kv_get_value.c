/**
 * @file kv_get_value.c
 * @brief Simple ANSI C Key-Value Parser.
 *
 * This file contains a parser function that extracts values associated with keys in a formatted
 * key-value file (e.g., "key=value" or "key: value").
 *
 * This code is released to the public domain by Brian Khuu in 2025.
 */

unsigned int kv_get_value(const char *str, const char *key, char *value, unsigned int value_max)
{
    for (; *str != '\0'; str++)
    {
#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
        while (*str == ' ' || *str == '\t')
        {
            str++;
        }
#endif

        /* Check For Key */
        for (int i = 0; *str != '\0' && key[i] != '\0'; i++, str++)
        {
            /* Key Mismatched. Skip Line */
            if (*str != key[i])
            {
                goto skip_line;
            }
        }

#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
        while (*str == ' ' || *str == '\t')
        {
            str++;
        }
#endif

        /* Check For Key Value Delimiter */
        if (*str != '=' && *str != ':')
        {
            goto skip_line;
        }
        str++;

#ifndef KV_PARSE_DISABLE_WHITESPACE_SKIP
        while (*str == ' ' || *str == '\t')
        {
            str++;
        }
#endif

        /* Copy Value To Buffer */
#ifndef KV_PARSE_DISABLE_QUOTED_STRINGS
        char quote = '\0';
#endif
        for (int i = 0; i < (value_max - 1); str++)
        {
            if (*str == '\0' || *str == '\r' || *str == '\n')
            {
                /* End Of Line. Trim trailing whitespace before returning the value. */
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
            else if (quote == '\0' && (*str == '\'' || *str == '"'))
            {
                /* Start Of Quoted String */
                quote = *str;
                continue;
            }
            else if (quote != '\0' && *(str - 1) != '\\' && *str == quote)
            {
                /* End Of Quoted String. Return Value */
                value[i] = '\0';
                return i;
            }
            else if (quote != '\0' && *(str - 1) == '\\' && *str == quote)
            {
                /* Escaped Quote Character In Quoted String */
                value[i - 1] = *str;
                continue;
            }
#endif

            value[i++] = *str;
        }

        /* Value too large for buffer. Don't return a value. */
        value[0] = '\0';
        return 0;

    skip_line:
        /* Search for start of next line */
        while (*str != '\n')
        {
            if (*str == '\0')
            {
                /* End of string. Key was not found. */
                return 0;
            }
            str++;
        }
    }

    /* End of string. Key was not found. */
    return 0;
}
