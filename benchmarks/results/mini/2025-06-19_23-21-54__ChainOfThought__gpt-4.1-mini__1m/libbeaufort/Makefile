
CC ?= cc
VALGRIND ?= valgrind

SRC = $(filter-out src/main.c, $(wildcard src/*.c))
OBJS = $(SRC:.c=.o)

OS = $(shell uname)

PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man

MAN_FILES = $(wildcard man/*.md)

BIN ?= beaufort
LIB_NAME ?= beaufort

VERSION_MAJOR = 0
VERSION_MINOR = 0
VERSION_PATCH = 2

TARGET_NAME = lib$(LIB_NAME)
TARGET_STATIC = $(TARGET_NAME).a
TARGET_DSOLIB = $(TARGET_NAME).so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
TARGET_DSO = $(TARGET_NAME).so

CFLAGS += -Iinclude -std=c99 -Wall -O2 -fvisibility=hidden -fPIC -pedantic
LDFLAGS += -o $(TARGET_DSOLIB) -shared -soname $(TARGET_DSO).$(VERSION_MAJOR)
CFLAGS += -DBEAUFORT_VERSION='"$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)"'
CFLAGS += -Ideps

ifeq ($(OS), Darwin)
	LDFLAGS += -lc -Wl,-install_name,$(TARGET_DSO)
endif

$(BIN): $(TARGET_STATIC) $(TARGET_DSO)
	$(CC) $(CFLAGS) $(SRC) src/main.c -o $(BIN)

install: $(BIN)
	install include/beaufort.h $(PREFIX)/include
	install $(TARGET_DSO) $(PREFIX)/lib
	install $(BIN) $(PREFIX)/bin
	install man/*.1 $(MANPREFIX)/man1
	install man/*.3 $(MANPREFIX)/man3

$(TARGET_STATIC): $(OBJS)
	ar crus $(TARGET_STATIC) $(OBJS)

$(TARGET_DSO): $(OBJS)
ifeq ("Darwin","$(OS)")
	$(CC) -shared $(OBJS) $(OSX_LDFLAGS) -o $(TARGET_DSOLIB)
	ln -s $(TARGET_DSOLIB) $(TARGET_DSO)
	ln -s $(TARGET_DSOLIB) $(TARGET_DSO).$(VERSION_MAJOR)
else
	$(CC) -shared $(OBJS) -o $(TARGET_DSOLIB)
	ln -s $(TARGET_DSOLIB) $(TARGET_DSO)
	ln -s $(TARGET_DSOLIB) $(TARGET_DSO).$(VERSION_MAJOR)
	strip --strip-unneeded $(TARGET_DSO)
endif

$(OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

test: $(OBJS) $(TARGET_STATIC) deps
	cc test.c deps/ok/libok.a $(TARGET_STATIC) $(CFLAGS) -o test-$(BIN)
	./test-$(BIN)

deps:
	make -C deps/ok

clean:
	make clean -C deps/ok
	rm -f $(OBJS)
	rm -f $(BIN)
	rm -f $(TARGET_STATIC)
	rm -f $(TARGET_DSO)
	rm -f $(TARGET_DSOLIB)
	rm -f *.so*
	rm -f test-$(BIN)

docs: $(MAN_FILES)

$(MAN_FILES):
	curl -# -F page=@$(@) -o $(@:%.md=%) http://mantastic.herokuapp.com

.PHONY: deps $(MAN_FILES)
