# libsniff

![libsniff logo](https://raw.githubusercontent.com/4thel00z/logos/master/libsniff.png)

## Motivation

Opening raw sockets in c is mad complicated yo.
This library encapsulates the heavy lifting for non-weird drivers and returns you a socket
that you can you can consume for a given interface name.

## Installation

Since this is a [clib]() library you can easily install it via:

```
clib install 4thel00z/libsniff
```

## Usage

```c
#include "../deps/libsniff/libsniff.h"
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
```

## License

This project is licensed under the GPL-3 license.
