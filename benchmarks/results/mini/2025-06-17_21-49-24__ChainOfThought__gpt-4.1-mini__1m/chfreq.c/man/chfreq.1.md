chfreq(1) -- Character frequency utility
=================================

## SYNOPSIS

`chfreq` \[-hV\] \[-f format\]

## OPTIONS

  -V, --version
      output program version

  -h, --help
      output help information

  -f [format]
      set output format (Default: '%c | %d')

## EXAMPLES

  ``
  $ echo -n aabbcc | chfreq
  a | 2
  b | 2
  c | 2
  ``

  ``
  $ echo -n 'kinkajous are awesome !'| chfreq
  k | 2
  i | 1
  n | 1
  a | 3
  j | 1
  o | 2
  u | 1
  s | 2
    | 3
  r | 1
  e | 3
  w | 1
  m | 1
  ! | 1
  ``

  ``
  $ echo -n werle | chfreq -f '%c(%d)'
  w(1)
  e(2)
  r(1)
  l(1)
  ``


## AUTHOR

  - Joseph Werle <joseph.werle@gmail.com>

## REPORTING BUGS

  - <https://github.com/jwerle/chfreq.c/issues>

## SEE ALSO

  - <https://github.com/jwerle/chfreq.c>

## LICENSE

MIT
