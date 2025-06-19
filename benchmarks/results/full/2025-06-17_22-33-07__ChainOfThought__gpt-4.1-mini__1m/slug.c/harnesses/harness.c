#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Declaration for slug from slug.h
char *slug(char *str);

// Implement minimal case_lower to avoid external dependencies, as slug uses it.
// From case.c:
#define modifier     ('a' - 'A')
#define is_lower(c)  ((c) >= 'a' && (c) <= 'z')
#define is_upper(c)  ((c) >= 'A' && (c) <= 'Z')

char *case_lower(char *str) {
  for (int i = 0, len = (int)strlen(str); i < len; i++) {
    if (is_upper(str[i])) {
      str[i] += modifier;
    }
  }
  return str;
}

// We redefine alphanum here because slug.c references case_lower and alphanum and trim
// but to keep the harness minimal and self-contained, we embed slug() here
// as provided in the source, plus needed case_lower and trim functions.

#include <stdio.h>

// copy from trim.c
static char *trim_left(char *str) {
  int len = (int)strlen(str);
  char *cur = str;

  while (*cur && isspace((unsigned char)*cur)) {
    ++cur;
    --len;
  }

  if (str != cur) {
    memmove(str, cur, (size_t)len + 1);
  }

  return str;
}

static char *trim_right(char *str) {
  int len = (int)strlen(str);
  if (len == 0) return str;
  char *cur = str + len - 1;

  while (cur != str && isspace((unsigned char)*cur)) {
    --cur;
    --len;
  }

  cur[isspace((unsigned char)*cur) ? 0 : 1] = '\0';

  return str;
}

char *trim(char *str) {
  trim_right(str);
  trim_left(str);
  return str;
}

// alphanum function copied from slug.c
static char *alphanum(char *str)
{
  // note that the loop uses strlen(str) each iteration.
  // So recalc length each iteration to avoid issues skipping chars on removal.
  for (int i = 0; str[i] != '\0'; /* i updated inside loop */) {
    if (isspace((unsigned char)str[i])) {
      str[i] = '-';
      i++;
    }
    else if (!isalnum((unsigned char)str[i]) && str[i] != '.') {
      memmove(&str[i], &str[i + 1], strlen(str) - i); // remove char, length shrink by 1
      // do not increment i, to check new char at i in next iteration
    } else {
      i++;
    }
  }
  return str;
}

// slug, incorporating case_lower, alphanum, and trim
char *slug(char *str)
{
  str = case_lower(str);
  str = alphanum(str);
  str = trim(str);

  return str;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Allocate buffer with an extra byte for null terminator
  char *buf = malloc(size + 1);
  if (!buf) return 0;

  memcpy(buf, data, size);
  buf[size] = '\0';

  // Call slug which modifies the string in-place
  slug(buf);

  // Free buffer
  free(buf);

  return 0;
}