install:
	clib install --dev

test:
	@$(CC) test.c -std=c99 -I src -I deps -o $@
	@./$@

.PHONY: test
