
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Wpedantic
LDLIBS=

SRC=main.c
OBJ=$(addprefix obj/, $(addsuffix .o, $(SRC)))

BIN=plumber

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

#-------------------------------------------------------------------------------

.PHONY: all clean install

all: $(BIN)

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

install: $(BIN)
	mkdir -p $(BINDIR)
	install -m 755 $^ $(DESTDIR)$(BINDIR)

#-------------------------------------------------------------------------------

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<
