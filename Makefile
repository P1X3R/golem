CC := clang

# Build mode (default = release)
MODE ?= release

COMMON_FLAGS = -Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE=200808L

ifeq ($(MODE),debug)
    SANITIZERS := address,undefined
    CFLAGS := $(COMMON_FLAGS) -O1 -g -fsanitize=$(SANITIZERS) -fno-omit-frame-pointer
    LDFLAGS := -fsanitize=$(SANITIZERS)
else ifeq ($(MODE),release)
    CFLAGS := $(COMMON_FLAGS) -O3 -march=native -DNDEBUG -flto
    LDFLAGS := -flto
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
