#include "../deps/libsniff.h"
#include <stdio.h>

int main() {
  int sock = libsniff_open_raw("wlan0mon");

  if (sock == -1) {
    fprintf(stderr, "something went wrong with libsniff_open_raw");
    return sock;
  }
  // Use sock with recv and a buffer...

  return 0;
}