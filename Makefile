
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Wpedantic
LDLIBS=

SRC=main.c pattern.c transform.c
OBJ=$(addprefix obj/, $(addsuffix .o, $(SRC)))
BIN=plumber

TEST_SRC=test.c pattern.c
TEST_OBJ=$(addprefix obj/, $(addsuffix .o, $(TEST_SRC)))
TEST_BIN=plumber-test

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

#-------------------------------------------------------------------------------

.PHONY: all test clean install

all: $(BIN)

test: $(TEST_BIN)
	./$<

clean:
	rm -f $(OBJ) $(BIN)
	rm -f $(TEST_OBJ) $(TEST_BIN)

install: $(BIN)
	install -D -m 755 $^ -t $(DESTDIR)$(BINDIR)

#-------------------------------------------------------------------------------

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(TEST_BIN): $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

obj/%.c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

obj/%.c.o: test/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<
