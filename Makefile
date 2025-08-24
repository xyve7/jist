CC = gcc 

CFLAGS = -g -Wall -Wextra -Werror -O0 -std=gnu11 -Isrc/include -Ilib/src/include -ldl -march=native -ljist -lmagic -lsqlite3
CFILES = $(shell find src -type f -name '*.c')
COBJS = $(patsubst src/%,obj/%,$(CFILES:.c=.o))

OUT = jist

.PHONY: all clean

bin/$(OUT): $(COBJS)
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(COBJS) -o $@

obj/%.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj bin

