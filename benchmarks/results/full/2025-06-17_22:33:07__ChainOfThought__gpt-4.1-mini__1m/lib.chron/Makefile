CC=gcc
CFLAGS=-g -pthread -Ideps -fPIC -Wall -Wextra -pedantic 
LDFLAGS=-shared -o
BIN=libchron.so

OBJFILES=$(wildcard src/*.c)
DEPS=$(wildcard deps/*/*.c)  

TESTS = $(patsubst %.c, %, $(wildcard t/*.c))

all:
	$(CC) $(CFLAGS) $(DEPS) $(OBJFILES) $(LDFLAGS) $(BIN)

clean:
	rm -f $(TARGET) $(BIN) $(WIN_BIN) main main.o 

test: 
	./scripts/test.bash 
	$(MAKE) clean

.PHONY: test clean 
