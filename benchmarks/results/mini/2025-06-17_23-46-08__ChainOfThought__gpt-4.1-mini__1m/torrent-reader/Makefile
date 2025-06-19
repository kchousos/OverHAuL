CONTRIB_DIR = ..

GCOV_OUTPUT = *.gcda *.gcno *.gcov 
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
SHELL  = /bin/bash
CC     = gcc
INCLUDES = $(shell echo deps/* | sed 's/^/-I/')
CCFLAGS = -g -O2 -Wall -Werror -W -fno-omit-frame-pointer -fno-common -fsigned-char $(GCOV_CCFLAGS) -I. $(INCLUDES) -Itests

#ifeq ($(OS),Windows_NT)
#    CCFLAGS += -D WIN32
#    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
#        CCFLAGS += -D AMD64
#    endif
#    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
#        CCFLAGS += -D IA32
#    endif
#else
#    UNAME_S := $(shell uname -s)
#    ifeq ($(UNAME_S),Linux)
#        CCFLAGS += -D LINUX
#    endif
#    ifeq ($(UNAME_S),Darwin)
#        CCFLAGS += -D OSX
#    endif
#    UNAME_P := $(shell uname -p)
#    ifeq ($(UNAME_P),x86_64)
#        CCFLAGS += -D AMD64
#    endif
#    ifneq ($(filter %86,$(UNAME_P)),)
#        CCFLAGS += -D IA32
#    endif
#    ifneq ($(filter arm%,$(UNAME_P)),)
#        CCFLAGS += -D ARM
#    endif
#endif


all: test
main.c:
	sh tests/make-tests.sh tests/test_*.c > main.c

test: main.c torrent_reader.o tests/test_torrent_reader.c tests/CuTest.c main.c deps/heapless-bencode/bencode.c 
	$(CC) $(CCFLAGS) -o $@ $^
	gcov torrent_reader.c
	./test

torrent_reader.o: torrent_reader.c
	$(CC) $(CCFLAGS) -c -o $@ $^

clean:
	rm -f main.c *.o test $(GCOV_OUTPUT)
