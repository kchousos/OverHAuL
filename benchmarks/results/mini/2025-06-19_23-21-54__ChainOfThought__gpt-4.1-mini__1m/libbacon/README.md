libbacon
========

A C implementation of the Baconian Cipher ! BACON

## about

Inspired by [bacon-cipher](https://github.com/mathiasbynens/bacon-cipher) !

Encode or decodes a message with each letter of the plaintext replaced by a group of five of the letters consisting of either an `A` or `B`. This replacement is a binary encoding and is done according to the alphabet of the [Baconian cipher](http://en.wikipedia.org/wiki/Bacon's_cipher).

The default translation table is shown below:

```
a   AAAAA   g     AABBA   n    ABBAA   t     BAABA
b   AAAAB   h     AABBB   o    ABBAB   u-v   BAABB
c   AAABA   i-j   ABAAA   p    ABBBA   w     BABAA
d   AAABB   k     ABAAB   q    ABBBB   x     BABAB
e   AABAA   l     ABABA   r    BAAAA   y     BABBA
f   AABAB   m     ABABB   s    BAAAB   z     BABBB
```

## install

[clib](https://github.com/clibs/clib):

```sh
$ clib install jwerle/libbacon
```

source:

```sh
$ git clone git@github.com:jwerle/libbacon.git
$ cd libbacon
$ make
$ make install
```

## usage

**Command line utility:**

*encode:*

```sh
$ echo abc | bacon --encode
AAAAAAAAABAAABA
```

```sh
$ { echo abc && echo def && echo ghi; }  | bacon --encode
AAAAAAAAABAAABA
AAABBAABAAAABAB
AABBAAABBBABAAA
```

*decode:*

```sh
$ echo AAAAAAAAABAAABA | bacon --decode
ABC
```

```sh
$ { echo 'AAAAAAAAABAAABA' && echo 'AAABBAABAAAABAB' && echo 'AABBAAABBBABAAA'; } | bacon --decode
ABC
DEF
GHI
```

**C Library:**

*encode:*

```c
#include <bacon.h>
#include <stdio.h>

int
main (void) {
  printf("%s\n", bacon_encode("foo bar", NULL));
  return 0;
}
```

...yields:

```
AABABABBABABBAB AAAABAAAAABAAAA
```

*decode:*

```c
#include <bacon.h>
#include <stdio.h>

int
main (void) {
  char *buf = "AA AA AA AAA BAAAB AAAABBA ABAAA.ABAB#AABBA@AABB+BABAA/AABAAAABAABABABAA"
    "B_ABBABBAAABBABABBB AABBBBBAAAABAAABBAABABAAB<BBAABBBABAABABABBA>BBABABB"
    "B";

  printf("%s\n", bacon_decode(buf, NULL));
  return 0;
}
```

...yields:

```
A B CD E F G H IIKL MNO PQRST UUWX YZ
```

## api

See [bacon.h](https://github.com/jwerle/libbacon/blob/master/include/bacon.h)

## license

MIT
