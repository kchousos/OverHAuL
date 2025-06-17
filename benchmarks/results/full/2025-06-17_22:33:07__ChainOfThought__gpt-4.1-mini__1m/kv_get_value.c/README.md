# kv_get_value.c

<versionBadge>![Version 1.0.1](https://img.shields.io/badge/version-1.0.1-blue.svg)</versionBadge>
[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CI/CD Status Badge](https://github.com/mofosyne/kv_get_value.c/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/mofosyne/kv_get_value/actions)

Simple ANSI C Key Value String Parser

Writeup about this mini project <https://briankhuu.com/blog/2025/01/30/simple-key-value-text-parser-in-c/>

If you need a kv parser that can be more easily modified. Consider <https://github.com/mofosyne/kv_parse.c>

## Get Value

This function searches for a specified key in a formatted key-value string
and extracts its corresponding value into the provided buffer.

Pros:
  * Self contained. Only stdio.h required for `kv_file_get_value()`
  * Minimise Size
  * Minimise Complexity

Cons:
  * Scan entire file for one key. This gets very inefficient for larger KV files. But practically very fast for small projects.
  * Not easy to parse INI file, you would need to read an INI section into it's own buffer then pass it to this function

```c
unsigned int kv_get_value(const char *str, const char *key, char *value, unsigned int value_max);
unsigned int kv_file_get_value(FILE *file, const char *key, char *value, unsigned int value_max);
```

This above will be able to read Bash Style key value files

```c
XDG_SESSION_DESKTOP=cinnamon
QT_QPA_PLATFORMTHEME=qt5ct
XDG_SESSION_TYPE=x11
```

But we can also interpret ':' and also skip whitespace. (Disable via `KV_PARSE_DISABLE_WHITESPACE_SKIP` compile flag).

```yaml
XDG_SESSION_DESKTOP   : cinnamon
QT_QPA_PLATFORMTHEME  : qt5ct
XDG_SESSION_TYPE      : x11
```

You can also escaped quote strings like in c (Disable via `KV_PARSE_DISABLE_QUOTED_STRINGS` compile flag).

```c
XDG_SESSION_DESKTOP="cinnamon"
QT_QPA_PLATFORMTHEME="qt5ct"
XDG_SESSION_TYPE="x11"
```

## Supported Features:
  - **Whitespace Skipping** (Disable via: `KV_PARSE_DISABLE_WHITESPACE_SKIP`): Ignores spaces and tabs around keys and values.
  - **Quoted String Support** (Disable via: `KV_PARSE_DISABLE_QUOTED_STRINGS`): Handles values enclosed in single (`'`) or double (`"`) quotes.
  - **Key Delimiters** : Supports both `=` and `:` as key-value separators.

# Usage Example:

## kv_get_value()

```c
char buffer[50];
unsigned int len = kv_get_value("username=admin\npassword=1234", "username", buffer, sizeof(buffer));
if (len > 0) {
    printf("Username: %s\n", buffer);
} else {
    printf("Key not found.\n");
}
```

## kv_file_get_value()

```c
char buffer[50];
FILE *temp = tmpfile();
fputs("username=admin\npassword=1234", temp);
rewind(temp);
unsigned int len = kv_file_get_value(temp, "username", buffer, sizeof(buffer));
if (len > 0) {
    printf("Username: %s\n", buffer);
} else {
    printf("Key not found.\n");
}
```
