# SixPack

[![Build & Test Status](https://github.com/aperezdc/sixpack/actions/workflows/test.yml/badge.svg?event=push)](https://github.com/aperezdc/sixpack/actions/workflows/test.yml)
[![Coverage Status](https://coveralls.io/repos/github/aperezdc/sixpack/badge.svg?branch=main)](https://coveralls.io/github/aperezdc/sixpack?branch=main)

Minimalist, [SAX](https://en.wikipedia.org/wiki/Simple_API_for_XML)-style,
single-file, parser for the [HiPack](http://hipack.org) serialization format
written in C99+.

Features:

* Small event-based API surface.
* Written in portable C99.
* Depends only on a few C standard library headers.
* *(Optional)*. May operate [without memory allocations](#zero-allocation-mode),
  making it suitable for small embedded devices.


## Sample Tool

Building `sixpack.c` with the `SIXPACK_MAIN` macro defined results in a
small testing program:

```
% cc -DSIXPACK_MAIN -o sixpack sixpack.c
% ./sixpack -h
Usage: ./sixpack [-vt] [files...]
Without files, parse standard input.
Options:
  -v  Show file paths as they are being processed.
  -t  Dump tree structure of parsed files.
%
```

Running `make sixpack` (or plainly `make`) compiles this program, too.


## Sample Document Tree Building

The [sixdom.hh](sixdom.hh) and [sixdom.cc](sixdom.cc) files show how to build
a document tree from the parser callbacks. This is implemented in C++20 for
the convenience of the containers (like `std::vector` or `std::unordered_map`
provided by the standard library. Included are both a wrapping API that makes
the bare C API more idiomatic, and the `DOMBuilder` itself.

A sample test program may be built with `make sixdompp`.


## Zero Allocation Mode

When built with the `SIXPACK_STATIC` macro defined, the parser uses a fixed
size buffer, and does away with the need for `malloc()`, `realloc()`, and
`free()`.

The size of the buffer is determined by the `SIXPACK_STATIC_BUFFER_SIZE`
macro. Its default size is `255`. One additional scratch byte is allocated
to zero-terminate strings.

In this mode the maximum *textual* length of numbers, strings, and
dictionary keys is limited by the buffer size in use.


## License

* [MIT](https://opensource.org/license/MIT)
* Code under `thirdparty/` may have different licensing terms, make sure to
  check [the separate README](thirdparty/README.md).
