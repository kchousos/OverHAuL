locale-string.c [![Build Status](https://travis-ci.org/mattn/locale-string.c.png?branch=master)](https://travis-ci.org/mattn/locale-string.c)
==========================

Convert locale string and utf-8 string

```c
#include "locale-string.h"
#include <stdlib.h>
#include <locale.h>

int
main(int argc, char* argv[]) {
  setlocale(LC_CTYPE, "");

  /* argv should be encoding as string depend on locale.
   * utf8_from_locale_alloc convert locale stirng
   * into utf8 stirng.
   */
  char* utf8 = utf8_from_locale_alloc(argv[1]);
  puts(utf8);

  /* Most of modern libraries are handle string as
   * utf-8, but filesystem should be encoding as string
   * depend on locale. utf8_to_locale_alloc convert
   * utf-8 string into locale string.
   */
  char* mbs = utf8_to_locale_alloc(utf8);
  FILE* fp = fopen(mbs, "w");
  fprintf(fp, "hello world\n");
  fclose(fp);

  free(utf8);
  free(mbs);
}
```

However, recently, we use utf-8 strings on UNIX OSs.
So you can write above with same meaning like below.
This code works well on windows too.

```c
#include "locale-string.h"
#include <stdlib.h>
#include <locale.h>

int
main(int argc, char* argv[]) {
  setlocale(LC_CTYPE, "");

  /* argv should be encoding as string depend on locale.
   * utf8_from_locale_alloc convert locale stirng
   * into utf8 stirng.
   */
  char* utf8 = UTF8_ALLOC(argv[1]);
  puts(utf8);

  /* Most of modern libraries are handle string as
   * utf-8, but filesystem should be encoding as string
   * depend on locale. utf8_to_locale_alloc convert
   * utf-8 string into locale string.
   */
  char* mbs = LOCALE_ALLOC(utf8);
  FILE* fp = fopen(mbs, "w");
  fprintf(fp, "hello world\n");
  fclose(fp);

  UTF8_FREE(utf8);
  LOCALE_FREE(mbs);
}
```

UTF8_ALLOC and LOCALE_ALLOC doesn't allocate memory on UNIX OSs.
