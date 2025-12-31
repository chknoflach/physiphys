CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99 -I/usr/local/include
LDFLAGS := -L/usr/local/lib
LDLIBS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC := src/main.c
BIN := app

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(BIN)

re: clean all

.PHONY: all clean re

