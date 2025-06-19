
/**
 * `encrypt.c' - libbeaufort
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <stdio.h>
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
beaufort_encrypt (const char *src, const char *key, char **mat) {
  char *enc = NULL;
  char ch = 0;
  char k = 0;
  size_t ksize = 0;
  size_t size = 0;
  size_t len = 0;
  size_t rsize = 0;
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
  enc = (char *) malloc(sizeof(char) * len + 1);

  if (NULL == enc) { return NULL; }

  for (; (ch = src[i]); ++i) {
    // reset
    needed = 1;

    // column with `ch' at top
    for (x = 0, y = 0; x < rsize; ++x) {
      if (ch == mat[y][x]) { needed = 1; break; }
      else { needed = 0; }
    }

    // if char not in top row
    // append current char
    if (0 == needed) {
      enc[size++] = ch;
      continue;
    }

    // determine char in `key'
    k = key[(j++) % ksize];

    // find row in column with `key[k]'
    for (y = 0; y < rsize; ++y) {
      if (k == mat[y][x]) { needed = 1; break; }
      else { needed = 0; }
    }

    // append char and decrement
    // unused modolu index if
    // not needed
    if (0 == needed) {
      enc[size++] = ch;
      j--;
      continue;
    }

    // append left char
    enc[size++] = mat[y][0];
  }

  enc[size] = '\0';

  return enc;
}
