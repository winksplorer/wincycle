OUT ?= wincycle
PREFIX ?= /usr/local

# Compiler and options. prefer clang if available, otherwise GCC
CC := $(shell command -v clang >/dev/null 2>&1 && echo clang || echo gcc)
CFLAGS = -O2 -std=c99 -Isrc -Wall -Wextra -pedantic -Wno-newline-eof -D_POSIX_C_SOURCE=200809L

CFILES = $(shell find src -type f -name '*.c')
OBJECTS = $(CFILES:.c=.o)

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(OBJECTS) -lX11 -o $(OUT)
    
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(OUT)

install:
	install -m 755 $(OUT) $(PREFIX)/bin
