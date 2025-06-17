CC      ?= cc
CP      ?= cp -f
RM      ?= rm -f
MKDIR   ?= mkdir -p
INSTALL ?= install

PREFIX  ?= /usr/local

#CFLAGS += -Wall -std=c99 -pedantic
CFLAGS += -Wall -std=c11 -pedantic

.PHONY:
all: test test.png

.PHONY: readme_update
readme_update:
	# Library Version (From clib package metadata)
	jq -r '.version' clib.json | xargs -I{} sed -i 's|<version>.*</version>|<version>{}</version>|' README.md
	jq -r '.version' clib.json | xargs -I{} sed -i 's|<versionBadge>.*</versionBadge>|<versionBadge>![Version {}](https://img.shields.io/badge/version-{}-blue.svg)</versionBadge>|' README.md

.PHONY: format
format:
	# pip install clang-format
	clang-format -i *.c
	clang-format -i *.h

.PHONY:
test: test.c sensor_smoother.c sensor_smoother.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o test test.c sensor_smoother.c -lm
	./test

test.png: test.gnuplot test.csv
	gnuplot test.gnuplot

#.PHONY:
#%.o: %.c
#	$(CC) $(DEP_FLAG) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY:
clean:
	rm -f test
