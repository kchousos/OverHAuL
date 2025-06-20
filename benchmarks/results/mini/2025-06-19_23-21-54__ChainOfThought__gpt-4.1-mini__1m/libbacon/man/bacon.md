bacon(1) -- Command line Baconian Cipher utility
=================================

## SYNOPSIS

`bacon` [-hV] [--encode|--decode] [--alphabet='ABC']

## OPTIONS

  -V, --version
      output program version

  -h, --help
      output help information

  --encode
      Encode stdin stream

  --decode
      Decode stdin stream

  --alphabet=[ALPHA]
      Cipher alphabet (Default: 'ABCDEFGHIKLMNOPQRSTUWXYZ')

## EXAMPLES

*encode:*

  $ echo abc | bacon --encode
  AAAAAAAAABAAABA

  $ { echo abc && echo def && echo ghi; }  | bacon --encode
  AAAAAAAAABAAABA
  AAABBAABAAAABAB
  AABBAAABBBABAAA

*decode:*

  $ echo AAAAAAAAABAAABA | bacon --decode
  ABC

  $ { echo 'AAAAAAAAABAAABA' && echo 'AAABBAABAAAABAB' && echo 'AABBAAABBBABAAA'; } | bacon --decode
  ABC
  DEF
  GHI

## AUTHOR

  - Joseph Werle <joseph.werle@gmail.com>

## REPORTING BUGS

  - <https://github.com/jwerle/libbacon/issues>

## SEE ALSO

  - <https://github.com/jwerle/libbacon>
  - <https://github.com/mathiasbynens/bacon-cipher>

## LICENSE

MIT
