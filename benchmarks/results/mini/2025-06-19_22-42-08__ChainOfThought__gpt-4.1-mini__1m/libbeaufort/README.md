libbeaufort
===========

A C implementation of the Beaufort Cipher

## about

## install

[clib](https://github.com/clibs/clib):

```sh
$ clib install jwerle/libbeaufort
```

source:

```sh
$ git clone git@github.com:jwerle/libbeaufort.git
$ cd libbeaufort
$ make
$ make install
```

## usage

*Command line utility:*

**encrypt:**

```sh
$ echo kinkajous are awesome | beaufort --encrypt --key=panda
5s0t06mtl 0yw Dhwxm1z
```

**decrypt:**

```sh
$ echo 5s0t06mtl 0yw Dhwxm1z | beaufort --decrypt --key=panda
kinkajous are awesome
```

*C Library:*

```c
#include <stdio.h>
#include <stdlib.h>
#include <beaufort.h>

int
main (void) {
  char *str = "bradley the kinkajou has a secret";
  char *key = "monkey";
  char *enc = beaufort_encrypt(str, key, NULL);
  printf("%s\n", enc); // BxD7tKo v66 uGz4D1q4 5Ev A mKAx9r
  char *dec = beaufort_decrypt(enc, key, NULL);
  printf("%s\n", dec); // bradley the kinkajou has a secret
  return 0;
}
```

## api

See [beaufort.h](https://github.com/jwerle/libbeaufort/blob/master/include/beaufort.h)

## license

MIT
