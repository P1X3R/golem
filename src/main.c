#include <stdint.h>
#include <stdio.h>

#include "bitboard.h"
#include "defs.h"
#include "luts.h"

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
  for (square_t sq = SQ_A1; sq <= SQ_H8; sq++) {
    const magic_t entry = ROOK_MAGICS[sq];
    const bitboard_t occupancy = bit(9);
    const bitboard_t attacks =
        SLIDING_ATTACKS_LUT[entry.offset +
                            (((occupancy & entry.mask) * entry.magic) >>
                             entry.shift)];

    print_bitboard(attacks);
  }
}
