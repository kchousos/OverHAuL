
/**
 * `tableau.c' - libbeaufort
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <beaufort.h>

static size_t
ssize (const char *str) {
  size_t size = 0;
  while ('\0' != str[size]) size++;
  return size;
}

char **
beaufort_tableau (const char *alpha) {
  size_t size = ssize(alpha);
  char **mat = NULL;
  int x = 0;
  int y = 0;
  int j = 0;

  mat = (char **) calloc(size + 1, sizeof(char *));

  if (NULL == mat) { return NULL; }

  for (;y < size; ++y) {
    mat[y] = (char *) calloc(size, sizeof(char));

    if (NULL == mat[y]) { return NULL; }

    for (x = 0, j = size; x < size; ++x, --j) {
      mat[y][x] = alpha[(j + y) % size];
    }

    mat[y][x] = '\0';
  }

  mat[y] = NULL;

  return mat;
}
