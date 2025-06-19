
/**
 * `main.c' - libbacon
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bacon.h>

enum { NO_OP, ENCODE_OP, DECODE_OP };

static void
usage () {
  fprintf(stderr, "usage: bacon [-hV] [--encode|--decode] [--alphabet='ABC']\n");
}

static void
help () {
  fprintf(stderr, "\noptions:\n");
  fprintf(stderr, "\n  --encode            Encode stdin stream");
  fprintf(stderr, "\n  --decode            Decode stdin stream");
  fprintf(stderr, "\n  --alphabet=[ALPHA]  Cipher alphabet (Default: 'ABCDEFGHIKLMNOPQRSTUWXYZ')");
  fprintf(stderr, "\n");
}

static char *
read_stdin () {
  size_t bsize = 1024;
  size_t size = 1;
  char buf[bsize];
  char *res = (char *) malloc(sizeof(char) * bsize);
  char *tmp = NULL;

  // memory issue
  if (NULL == res) { return NULL; }

  // cap
  res[0] = '\0';

  // read
  if (NULL != fgets(buf, bsize, stdin)) {
    // store
    tmp = res;
    // resize
    size += (size_t) strlen(buf);
    // realloc
    res = (char *) realloc(res, size);

    // memory issues
    if (NULL == res) {
      free(tmp);
      return NULL;
    }

    // yield
    strcat(res, buf);

    return res;
  }

  free(res);

  return NULL;
}

int
main (int argc, char **argv) {
  char *buf = NULL;
  char *alpha = NULL;
  char *out = NULL;
  int op = NO_OP;

  // emit usage with empty arguments
  if (1 == argc) { return usage(), 1; }

  // parse opts
  {
    int i = 0;
    char *opt = NULL;
    char tmp = 0;

    opt = *argv++; // unused

    while ((opt = *argv++)) {

      // flags
      if ('-' == *opt++) {
        switch (*opt++) {
          case 'h':
            return usage(), help(), 0;

          case 'V':
            fprintf(stderr, "%s\n", BACON_VERSION);
            return 0;

          case '-':
            if (0 == strcmp(opt, "encode")) { op = ENCODE_OP;}
            if (0 == strcmp(opt, "decode")) { op = DECODE_OP;}
            if (0 == strncmp(opt, "alphabet=", 8)) {
              for (i = 0; i < 9; ++i) tmp = *opt++;
              alpha = opt;
            }
            break;

          default:
            // error
            fprintf(stderr, "unknown option: `%s'\n", opt);
            usage();
            return 1;
        }
      }
    }
  }

#define OP(name) {                               \
  buf = read_stdin();                            \
  if (NULL == buf) { return 1; }                 \
  out = bacon_ ## name(buf, alpha);              \
  printf("%s\n", out);                           \
  do {                                           \
    buf = read_stdin();                          \
    if (NULL == buf) { break; }                  \
    out = bacon_ ## name(buf, alpha);            \
    printf("%s\n", out);                         \
  } while (NULL != buf);                         \
}

switch (op) {
  case ENCODE_OP:
    if (1 == isatty(0)) { return 1; }
    else if (ferror(stdin)) { return 1; }
    else { OP(encode); }
    return 0;

  case DECODE_OP:
    if (1 == isatty(0)) { return 1; }
    else if (ferror(stdin)) { return 1; }
    else { OP(decode); }
    return 0;

  case NO_OP:
  default:
    return usage(), 1;
}

#undef OP
}
