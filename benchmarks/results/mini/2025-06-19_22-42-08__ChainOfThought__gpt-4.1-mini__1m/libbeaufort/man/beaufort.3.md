beaufort(3) -- Beaufort cipher functions
=================================

## LIBRARY

  libbeaufort (-lbeaufort)

## SYNOPSIS

  `#include <beaufort.h>`

  char *
  `beaufort_encrypt` (const char *src, const char *key, char **tableau);

  char *
  `beaufort_decrypt` (const char *str, const char *key, char **tableau);

  char **
  `beaufort_tableau` (const char *alphabet);

  #define `BEAUFORT_ALPHA` "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

## DESCRIPTION

  The function `beaufort_encrypt()` encrypts plaintext using the beaufort cipher
  and an alphabet table if provided. The function will fallback to a table
  generated using the default alphabet defined by `BEAUFORT_ALPHA`.

  The function `beaufort_decrypt()` decrypts plaintext using the beaufort cipher
  and an alphabet table if provided. The function will fallback to a table
  generated using the default alphabet defined by `BEAUFORT_ALPHA`.


  The function `beaufort_tableau()` constructs a tableau of rows and columns
  from a given alphabet using the following forumula to determine the row
  index in each column during creation:


  `(j + y) % size`

  where `size = strlen(alpha)`, `j = size - 1`, and * `y = y + 1`.

## RETURN VALUES

  The functions `beaufort_encrypt()` and `beaufort_decrypt()` each
return a pointer and should be freed with `free()`.

  The function `beaufort_tableau()` returns a pointer to a pointer and
thus each element in the array should be freed with `free()`.

## AUTHOR

  - Joseph Werle <joseph.werle@gmail.com>

## REPORTING BUGS

  - <https://github.com/jwerle/libbeaufort/issues>

## SEE ALSO

  - <https://github.com/jwerle/libbeaufort>

## LICENSE

MIT
