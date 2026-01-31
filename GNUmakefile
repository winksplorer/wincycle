OUT ?= wincycle
PREFIX ?= /usr/local

CFLAGS = -O2 -std=c99 -Isrc -Wall -Wextra -pedantic -Werror -D_POSIX_C_SOURCE=200809L

SRC := wincycle.c

all: $(OUT)

$(OUT):
	$(CC) $(CFLAGS) $(SRC) -lX11 -o $@

clean:
	rm -f $(OBJECTS) $(OUT)

install:
	install -m 755 $(OUT) $(PREFIX)/bin

.PHONY: all $(OUT) clean install