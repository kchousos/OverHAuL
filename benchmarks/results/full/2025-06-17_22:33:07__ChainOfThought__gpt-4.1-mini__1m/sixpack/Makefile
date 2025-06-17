PREFIX  ?= /usr/local
BINDIR  ?= $(PREFIX)/bin
DESTDIR ?=

all: sixpack

sixpack: sixpack.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DSIXPACK_MAIN $(LDFLAGS) -o $@ $<

sixdompp: sixdom.cc sixpack.o
	$(CXX) $(CXXFLAGS) -std=c++20 $(CPPFLAGS) -Ithirdparty -DSIXDOM_MAIN $(LDFLAGS) -o $@ $^

clean:
	$(RM) sixpack sixpack.o
	$(RM) sixdompp sixdom.o

install: sixpack
	install -dm755 $(DESTDIR)$(BINDIR)
	install -m755 $< $(DESTDIR)$(BINDIR)

test: sixpack
	@./tools/run-tests

.PHONY: install test
