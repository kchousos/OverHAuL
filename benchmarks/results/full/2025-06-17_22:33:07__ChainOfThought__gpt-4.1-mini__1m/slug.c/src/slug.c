
//
// slug.c
//
// Copyright (c) 2013 Henrik Kjelsberg
// MIT licensed
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "case.h"
#include "trim.h"
#include "slug.h"

#define separator '-'

char *alphanum(char *str)
{
  for (int i = 0, len = strlen(str); i < len; i++) {
    if (isspace(str[i])) str[i] = separator;
    else if (!isalnum(str[i]) && str[i] != '.') {
      memmove(&str[i], &str[i + 1], strlen(str) - i);
    }
  }
  return str;
}

char *slug(char *str)
{
  str = case_lower(str);
  str = alphanum(str);
  str = trim(str);

  return str;
}

