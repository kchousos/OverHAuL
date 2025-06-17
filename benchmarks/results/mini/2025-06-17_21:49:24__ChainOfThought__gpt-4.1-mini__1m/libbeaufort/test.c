
/**
 * `test.c' - libbeaufort
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ok/ok.h>
#include <beaufort.h>

#define test(name, e) {                   \
  if (!(e)) {                             \
    fprintf(stderr, "failed: %s\n", # e); \
    return 1;                             \
  } else {                                \
    ok(name);                             \
  }                                       \
}

#define texpect(fn, a, b) {              \
  printf("(%s): ", # fn);                \
  test(a, 0 == strcmp(fn(a, ALPHA), b)); \
}

static int
test_encrypt ();

static int
test_decrypt ();

static int
test_tableau ();

static char *monkey = NULL;
static char *monkey_s = NULL;

static char *goodman = NULL;
static char *goodman_s = NULL;

static char *groove = NULL;
static char *groove_s = NULL;

int
main (void) {
  test("encrypt suite", 0 == test_encrypt());
  test("decrypt suite", 0 == test_decrypt());
  test("tableau suite", 0 == test_tableau());
  ok("beaufort");
  return 0;
}

static int
test_encrypt () {

  monkey_s = "kinkajous are awesome";
  monkey = beaufort_encrypt(monkey_s, "monkey", NULL);
  assert(monkey);
  assert(0 == strcmp(monkey, "26004Fyuv AnK Cs9sqC8"));

  goodman_s = "the \nbig \nlebowski";
  goodman = beaufort_encrypt(goodman_s, "goodman", NULL);
  assert(goodman);
  assert(0 == strcmp(goodman, "n7A \n24u \n22D0huq5"));

  groove_s = "d4nc3 t0 th3 mus!c :D";
  groove = beaufort_encrypt(groove_s, "groove", NULL);
  assert(groove);
  assert(0 == strcmp(groove, "3n1Cs lg y7l 9ko!F :b"));

  return 0;
}

static int
test_decrypt () {
  char *dec = NULL;

  dec = beaufort_decrypt(monkey, "monkey", NULL);
  assert(dec);
  assert(0 == strcmp(dec, monkey_s));
  free(dec);

  dec = beaufort_decrypt(goodman, "goodman", NULL);
  assert(dec);
  assert(0 == strcmp(dec, goodman_s));
  free(dec);

  dec = beaufort_decrypt(groove, "groove", NULL);
  assert(dec);
  assert(0 == strcmp(dec, groove_s));
  free(dec);

  return 0;
}

static int
test_tableau () {

#define tfree(m) {                    \
  for (int i = 0; m[i]; ++i) {        \
    if (NULL != m[i]) { free(m[i]); } \
  }                                   \
}

  {
    char **mat = beaufort_tableau(BEAUFORT_ALPHA);
    assert(mat);

    {
      int x = 0;
      int y = 0;
      size_t l = strlen(mat[0]);
      for (; y < l; ++y) {
        for (x = 0; x < l; ++x) {
          printf("%c", mat[y][x]);
          if (x < l - 1) {
            printf(" | ");
          }
        }
        printf("\n");
      }
    }

    tfree(mat);
  }

  {
    char **mat = beaufort_tableau("abc");
    assert(mat);

    assert('a' == mat[0][0]);
    assert('c' == mat[0][1]);
    assert('b' == mat[0][2]);

    assert('b' == mat[1][0]);
    assert('a' == mat[1][1]);
    assert('c' == mat[1][2]);

    assert('c' == mat[2][0]);
    assert('b' == mat[2][1]);
    assert('a' == mat[2][2]);

    tfree(mat);
  }

#undef tfree

  return 0;
}
