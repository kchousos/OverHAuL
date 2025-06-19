#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "bacon.h"

/* Implementation of bacon_encode */
char *
bacon_encode (const char *src, const char *alpha) {
  char *enc = (char *) malloc(sizeof(char));
  char ch = 0;
  size_t size = 0;
  size_t len = (size_t) strlen(src);
  size_t alen = 0;
  int custom = 0;
  int i = -1; // source index
  int sep = 0;
  int idx = -1;

  if (NULL == enc) { return NULL; }

  // use default
  if (NULL == alpha) {
    alpha = BACON_ALPHA;
  } else { custom = 1; }

  // alpha length
  alen = (size_t) strlen(alpha);

  // parse and encode
  while ((++i) < len) {

    ch = toupper(src[i]);

    // apply char replacements
    // if not custom alpha
    if (0 == custom) {
      switch (ch) {
        case 'J': ch = 'I'; break;
        case 'V': ch = 'U'; break;
      }
    }

    // search alphabet for index
    {
      char c = 0;

      // reset
      idx = -1;

      // search
      while ((++idx) < alen) {
        c = toupper(alpha[idx]);
        if (ch == c) { goto found; }
      }

      // not found
      idx = -1;
    }
found:

    // found
    if (idx > -1) {
      // needs space
      if (1 == sep) {
        enc[size++] = ' ';
      }

      // encode
      enc[size++] = idx & 0x10 ? BACON_B : BACON_A;
      enc[size++] = idx & 0x08 ? BACON_B : BACON_A;
      enc[size++] = idx & 0x04 ? BACON_B : BACON_A;
      enc[size++] = idx & 0x02 ? BACON_B : BACON_A;
      enc[size++] = idx & 0x01 ? BACON_B : BACON_A;

      // reset
      sep = 0;
    } else if (i > 0) {
      // need space expect on first char
      sep = 1;
    }
  }

  // cap
  enc[size] = '\0';

  return enc;
}

/* Implementation of bacon_decode */
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

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }

    // Copy fuzz input to a null-terminated string buffer
    char *input = (char *)malloc(size + 1);
    if (!input) return 0;

    memcpy(input, data, size);
    input[size] = '\0';

    // Call bacon_encode with default alphabet (NULL)
    char *encoded = bacon_encode(input, NULL);
    if (encoded) {
        free(encoded);
    }

    // Call bacon_decode with same input
    char *decoded = bacon_decode(input, NULL);
    if (decoded) {
        free(decoded);
    }

    free(input);
    return 0;
}