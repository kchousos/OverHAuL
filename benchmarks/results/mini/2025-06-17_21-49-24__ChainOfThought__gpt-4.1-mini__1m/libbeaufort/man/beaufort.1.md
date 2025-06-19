beaufort(1) -- Command line Beaufort Cipher utility
=================================

## SYNOPSIS

`beaufort` \[-hV\] \[options\]

## OPTIONS

  `-V, --version`
      output program version

  `-h, --help`
      output help information

  `--encrypt`
      encrypt stdin stream

  `--decrypt`
      decrypt stdin stream

  `--key=[key]`
      cipher key (required)

  `--alphabet=[alpha]`
    cipher tableau alphabet (Default: 'abcdefghijklmnopqrstuvwxyz')

## EXAMPLES

  **encrypt:**

  ```
  $ echo kinkajous are awesome | beaufort --encrypt --key=panda
  5s0t06mtl 0yw Dhwxm1z
  ```

  **decrypt:**

  ```
  $ echo 5s0t06mtl 0yw Dhwxm1z | beaufort --decrypt --key=panda
  kinkajous are awesome
  ```

## AUTHOR

  - Joseph Werle <joseph.werle@gmail.com>

## REPORTING BUGS

  - <https://github.com/jwerle/libbeaufort/issues>

## SEE ALSO

  - <https://github.com/jwerle/libbeaufort>

## LICENSE

MIT
