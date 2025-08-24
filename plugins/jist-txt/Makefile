CC = gcc 
CFLAGS = -shared -Wall -Wextra -Werror -O2 -std=gnu11 -Isrc/include -march=native -I/home/xyve/Development/jist/lib/src/include -ljist
CFILES = $(shell find src -type f -name '*.c')
COBJS = $(patsubst src/%,obj/%,$(CFILES:.c=.o))

OUT = jist-txt.so 

.PHONY: all clean

bin/$(OUT): $(COBJS)
	mkdir -p "$$(dirname $@)"
	$(CC) -shared $(CFLAGS) $(COBJS) -o $@

obj/%.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj bin
