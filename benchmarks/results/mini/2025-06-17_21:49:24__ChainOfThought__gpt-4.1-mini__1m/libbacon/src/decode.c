
/**
 * `decode.c' - libbacon
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <bacon.h>

char *
bacon_decode (const char *src, const char *alpha) {
  char *dec = (char *) malloc(sizeof(char));
  char buf[5];
  char ch = 0;
  size_t size = 0;
  size_t len = (size_t) strlen(src);
  size_t alen = 0;
  size_t bsize = 0; // buffer size
  int i = -1; // source index
  int sep = 0;
  int idx = -1;
  int custom = 0;

  if (NULL == dec) { return NULL; }

  // use default
  if (NULL == alpha) {
    alpha = BACON_ALPHA;
  } else { custom = 1; }

  // alpha length
  alen = (size_t) strlen(alpha);

  // parse and decode
  while ((++i) < len) {
    // read symbol and convert
    // to uppercase just in case
    ch = toupper(src[i]);

    // store symbols in buffer
    if (BACON_A == ch || BACON_B == ch) {
      buf[bsize++] = ch;
    } else {
      // oob - needs space
      sep = 1;
    }

    if (5 == bsize) {
      // accumulate
      idx = (
          (buf[0] == BACON_A ? 0 : 0x10) +
          (buf[1] == BACON_A ? 0 : 0x08) +
          (buf[2] == BACON_A ? 0 : 0x04) +
          (buf[3] == BACON_A ? 0 : 0x02) +
          (buf[4] == BACON_A ? 0 : 0x01) 
       );

      // append space if needed and
      // is not first char yieled
      if (1 == sep && size > 0) {
        dec[size++] = ' ';
      }

      // append char from alphabet
      // uppercased
      dec[size++] = toupper(alpha[idx]);

      // reset
      bsize = 0;
      sep = 0;
    }
  }

  // cap
  dec[size] = '\0';

  return dec;
}
