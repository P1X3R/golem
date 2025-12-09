CC := gcc

# Build mode (default = release)
MODE ?= release

ifeq ($(MODE),debug)
    CFLAGS := -O1 -g -Wall -Wextra -Wpedantic -std=c99 \
              -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS := -fsanitize=address,undefined
else ifeq ($(MODE),release)
    CFLAGS := -O3 -march=native -Wall -Wextra -Wpedantic -std=c99 -DNDEBUG
    LDFLAGS :=
else
    $(error Unknown MODE '$(MODE)' (expected 'debug' or 'release'))
endif

SRC := $(filter-out src/bake.c src/main.c, $(wildcard src/*.c))
OBJ := $(SRC:.c=.o)

TARGETS := golem bake

all: golem

golem: src/main.o $(OBJ)
	$(CC) $^ -o $@.out $(LDFLAGS)

bake: src/bake.o $(OBJ)
	$(CC) $^ -o $@.out $(LDFLAGS)

# Pattern rule for .c → .o
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) src/main.o src/bake.o golem.out bake.out

.PHONY: all clean golem bake
