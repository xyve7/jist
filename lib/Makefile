CC = gcc 
CFLAGS = -Wall -Wextra -Werror -O2 -std=gnu11 -Isrc/include -march=native
CFILES = $(shell find src -type f -name '*.c')
COBJS = $(patsubst src/%,obj/%,$(CFILES:.c=.o))

OUT = libjist.so 

.PHONY: all clean

bin/$(OUT): $(COBJS)
	mkdir -p "$$(dirname $@)"
	$(CC) -shared $(CFLAGS) $(COBJS) -o $@

obj/%.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj bin
