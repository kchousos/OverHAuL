# kv_parse.c

<versionBadge>![Version 1.1.1](https://img.shields.io/badge/version-1.1.1-blue.svg)</versionBadge>
[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI/CD Status Badge](https://github.com/mofosyne/kv_get_value.c/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/mofosyne/kv_get_value/actions)

Composible ANSI C Key Value String Parser

If you need a kv parser that is simpler and single function... then consider <https://github.com/mofosyne/kv_get_value.c>

Compared to my other simple kv implementation above. This implementation is a bit more flexible 
and efficient at handling many key value pairs as well as providing opportunities for the users
to more easily modify the code to support other various key value formats (ini, etc...)

For example handling ini files, where you can add an additional check in between to parse out
the ini sections `[sections]` before handling the key value pairs. To do so you could try using
`kv_parse_buffer_check_section()` or `kv_parse_check_section()` to parse out the sections that is
also provided in this repository as a convenience function for you.

As before, this implementation will not try to parse out the value output to minimise complexity...
but this will at least handle the most annoying bits about handling key value pairs such as callbacks 
and dynamic allocation.

There is plenty of other C ini parser out there like <https://toml.io/en/> if you just need a
configuration language. For more complex INI style parsing (ergo section support) do consider:

* <https://github.com/clibs/inih> : Used by clib themselves, relatively simple. Uses callbacks.
* <https://github.com/madmurphy/libconfini> : More complex and can parse typed data in a multi line manner. Uses Callbacks.

Overall this kv parser can parse Bash Style key value files

```c
XDG_SESSION_DESKTOP=cinnamon
QT_QPA_PLATFORMTHEME=qt5ct
XDG_SESSION_TYPE=x11
```

But we can also interpret ':'

```yaml
XDG_SESSION_DESKTOP   : cinnamon
QT_QPA_PLATFORMTHEME  : qt5ct
XDG_SESSION_TYPE      : x11
```

We also support escaped quote strings like in c

```c
XDG_SESSION_DESKTOP="cinnamon"
QT_QPA_PLATFORMTHEME="qt5ct"
XDG_SESSION_TYPE="x11"
```

The key design properties that my implementation has:
* Pros:
    - No Malloc
    - No Callbacks
    - Simple and predictable API using FILE pointers and static buffers.
    - Thread safe. No global variables required.
    - Lightweight and composible. This keeps it small and easy to modify.
* Cons:
    - Limited scalability. At least the value buffer can be adjusted for each key call. But intentionally still using a fixed buffer.
    - We don't use callbacks with mallocs etc... this does lead to less efficiency. But intentionally chose to keep this project simple.
    - During key search, the function repeatedly use fseek() which slows down parsing. This can be sped up with hash maps.
    - No error handling. To keep this simple, you will need to add your own error logging such as returning an error code.
      But comments should make it easy to identify what to add.
    - No multiline handling. To control complexity level of the parser... multiline support is intentionally dropped.
      (But due to the composible design, it will be easier for developers to replace the value reader with their own implementation.)

**Pull Requests Appreciated**

## Buffered API

```c
char *kv_parse_buffer_next_line(char *str, size_t line_count);
char *kv_parse_buffer_check_key(char *str, const char *key);
size_t kv_parse_buffer_get_value(char *str, char *value, size_t value_max);
```

Examples: 

```c
int kv_buffer_parse(char *input, const char *key, char *value, unsigned int value_max)
{
    for (unsigned int line = 0; (input = kv_parse_buffer_next_line(input, line)) != NULL; line++)
    {
        char *input_value = NULL;
        if ((input_value = kv_parse_buffer_check_key(input, key)) != NULL)
        {
            int ret = kv_parse_buffer_get_value(input_value, value, value_max);
            return ret;
        }
    }
    return 0;
}
```

## FILE API

```c
bool kv_parse_next_line(FILE *file, size_t line_count);
bool kv_parse_check_key(FILE *file, const char *key);
size_t kv_parse_get_value(FILE *file, char *value, size_t value_max);
```

Examples:

```c
int kv_file_parse(FILE *file, const char *key, char *value, unsigned int value_max)
{
    rewind(file);
    for (unsigned int line = 0; kv_parse_next_line(file, line); line++)
    {
        if (kv_parse_check_key(file, key))
        {
            int ret = kv_parse_get_value(file, value, value_max);
            return ret;
        }
    }
    return 0;
}
```
