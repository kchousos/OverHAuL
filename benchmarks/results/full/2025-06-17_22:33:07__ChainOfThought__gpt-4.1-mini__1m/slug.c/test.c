
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "src/slug.h"

int main(int argc, char **argv) {

  char foo[] = "foo bar";
  assert(0 == strcmp("foo-bar", slug(foo)));

  char bar[] = "bar is closed!";
  assert(0 == strcmp("bar-is-closed", slug(bar)));

  char href[] = "Example.com";
  assert(0 == strcmp("example.com", slug(href)));

  char symbol[] = "@hkjels";
  assert(0 == strcmp("hkjels", slug(symbol)));

  return 0;
}

