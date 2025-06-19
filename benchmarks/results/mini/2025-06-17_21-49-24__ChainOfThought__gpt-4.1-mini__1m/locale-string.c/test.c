#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include "locale-string.h"

#define TEST(name) \
  static void test_##name(void)

#define RUN_TEST(test) \
  test_count++; \
  test_##test(); \
  puts("  " #test);

static int test_count = 0;

TEST (convert) {
  setlocale(LC_CTYPE, "");
  const char* ptr = "こんにちわ世界";
  char* mbs = utf8_to_locale_alloc(ptr);
  assert(NULL != mbs);
  char* utf8 = utf8_from_locale_alloc(mbs);
  assert(NULL != utf8);
  assert(0 == strcmp(ptr, utf8));
}

int
main () {
  clock_t start = clock();

  printf("\n");

  RUN_TEST(convert);

  printf("\n  %d tests passed in %.5fs\n\n"
    , test_count
    , (float) (clock() - start) / CLOCKS_PER_SEC);

  return 0;
}
