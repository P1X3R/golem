#include <stdint.h>
#include <stdio.h>

#include "bitboard.h"
#include "zobrist.h"

void print_bitboard(uint64_t bb) {
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      putchar((bb & bit(to_square(rank, file))) ? '1' : '.');
      putchar(' ');
    }
    putchar('\n');
  }
  putchar('\n');
}

int main(int argc, char* argv[]) {
  init_zobrist_tables();

  print_bitboard(ZOBRIST_COLOR);
}
