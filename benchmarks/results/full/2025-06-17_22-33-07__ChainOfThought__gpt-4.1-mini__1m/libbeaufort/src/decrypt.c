
/**
 * `decrypt.c' - libbeaufort
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <stdlib.h>
#include <ctype.h>
#include <beaufort.h>

static size_t
ssize (const char *str) {
  size_t size = 0;
  while ('\0' != str[size]) size++;
  return size;
}

char *
beaufort_decrypt (const char *src, const char *key, char **mat) {
  char *dec = NULL;
  char ch = 0;
  char k = 0;
  size_t ksize = 0;
  size_t size = 0;
  size_t rsize = 0;
  size_t len = 0;
  int i = 0;
  int x = 0;
  int y = 0;
  int j = 0;
  int needed = 1;

  if (NULL == mat) {
    mat = beaufort_tableau(BEAUFORT_ALPHA);
    if (NULL == mat) { return NULL; }
  }

  ksize = ssize(key);
  len = ssize(src);
  rsize = ssize(mat[0]);
  dec = (char *) malloc(sizeof(char) * len + 1);

  if (NULL == dec) { return NULL; }

  for (; (ch = src[i]); ++i) {
    needed = 1;

    // find column with char
    for (y = 0; y < rsize; ++y) {
      if (ch == mat[y][0]) { needed = 1; break; }
      else { needed = 0; }
    }

    // if not needed append
    // char and continue
    if (0 == needed) {
      dec[size++] = ch;
      continue;
    }

    // determine char in `key'
    k = key[(j++) % ksize];

    for (x = 0; x < rsize; ++x)  {
      if (k == mat[y][x]) { needed = 1; break; }
      else { needed = 0; }
    }

    // append current char if not
    // needed and decrement unused
    // modulo index
    if (0 == needed) {
      dec[size++] = ch;
      j--;
      continue;
    }

    dec[size++] = mat[0][x];
  }

  dec[size] = '\0';

  return dec;
}
