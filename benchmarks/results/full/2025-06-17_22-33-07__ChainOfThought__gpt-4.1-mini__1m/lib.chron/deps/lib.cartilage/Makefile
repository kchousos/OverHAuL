CC=gcc
CFLAGS=-g -fPIC -Wall -Wextra -pedantic -std=c17
LDFLAGS=-shared -o
 
BIN=libcartilage.so

OBJFILES=$(wildcard src/*.c)

TESTS = $(patsubst %.c, %, $(wildcard t/*.c))

all: 
	$(CC) $(CFLAGS) $(OBJFILES) $(LDFLAGS) $(BIN)
 
clean:
	rm -f $(TARGET) $(BIN) $(WIN_BIN) main main.o 

test: 
	./scripts/test.bash 
	$(MAKE) clean

.PHONY: test clean 
