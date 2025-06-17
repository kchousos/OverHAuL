
CC ?= gcc
CFLAGS += -std=c99 -Wall -Wextra -Ideps
DEPS := $(wildcard deps/*/*.c)
OBJS := occurrences.o $(DEPS:.c=.o)

.DEFAULT_GOAL := validate

test: test.o $(OBJS)

occurrences.o: occurrences.c occurrences.h

deps: package.json
	clib install --dev
	@touch $@

validate: test
	./$<

clean:
	rm -f $(OBJS) occurrences.o test.o test

.PHONY: validate clean
