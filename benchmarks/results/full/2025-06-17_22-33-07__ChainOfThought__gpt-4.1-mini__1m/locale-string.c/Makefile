
AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
CFLAGS = -std=c99 -Wall -Wextra -U__STRICT_ANSI__

SRC = locale-string.c
HEADERS = locale-string.h
OBJS = $(SRC:.c=.o)


all: clean test liblocale-string.a

liblocale-string.a: $(OBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

test: test.o $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)
	@./$@

clean:
	rm -f test $(OBJS)

install: liblocale-string.a
	cp -f liblocale-string.a $(PREFIX)/lib/liblocale-string.a
	cp -f src/locale-string.h $(PREFIX)/include/locale-string.h

uninstall:
	rm -f $(PREFIX)/lib/liblocale-string.a
	rm -f $(PREFIX)/include/locale-string.h

.PHONY: test clean include uninstall
