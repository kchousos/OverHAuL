SRC = $(wildcard *.c)
OBJS = $(SRC:.c=.o)

CFLAGS = -std=c99 -Wall

default: $(OBJS)

clean:
	rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: default clean
