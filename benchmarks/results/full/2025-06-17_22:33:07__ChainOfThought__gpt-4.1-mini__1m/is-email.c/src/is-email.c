
//
// is-email.c
//
// Copyright (c) 2013 Stephen Mathieson
// MIT licensed
//

#include <string.h>
#include <stdlib.h>
#include "is-email.h"

int is_email(const char *str) {
  int at = strrchr(str, '@') - str;
  // @bar.com
  if (at <= 0) return -1;

  int dot = strrchr(str, '.') - str;
  // whatever.@com
  if (at > dot) return -1;
  // foo@.bar
  if (at + 1 == dot) return -1;

  int delta = strlen(str) - dot - 1;
  // a@b.c
  if (3 > delta) return -1;

  return 0;
}
