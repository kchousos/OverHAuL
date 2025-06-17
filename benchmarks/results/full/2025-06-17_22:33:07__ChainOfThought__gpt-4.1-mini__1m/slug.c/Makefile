
SRC = $(wildcard src/*.c) $(wildcard deps/*.c)

test: test.c $(SRC)
	$(CC) -std=c99 $^ -o test-slug -Ideps -Isrc
	./test-slug

.PHONY: test

