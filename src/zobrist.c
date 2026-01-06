#include "zobrist.h"

#include <stdint.h>

#include "defs.h"
#include "misc.h"

uint64_t prng_state = 0x9e3779b97f4a7c15;

uint64_t ZOBRIST_PIECES[NR_OF_COLORS][NR_OF_PIECE_TYPES][NR_OF_SQUARES];
uint64_t ZOBRIST_COLOR;
uint64_t ZOBRIST_CASTLING_RIGHTS[16];  // 2^4 = 16
uint64_t ZOBRIST_EP_FILE[NR_OF_ROWS];

void init_zobrist_tables(void) {
  for (color_t color = 0; color < 2; color++) {
    for (square_t sq = SQ_A1; sq <= SQ_H8; sq++) {
      for (piece_t piece = PT_PAWN; piece <= PT_KING; piece++) {
        ZOBRIST_PIECES[color][piece][sq] = random_u64();
      }
    }
  }

  ZOBRIST_COLOR = random_u64();

  for (uint8_t rights_combination = 0; rights_combination < 16;
       rights_combination++) {
    ZOBRIST_CASTLING_RIGHTS[rights_combination] = random_u64();
  }

  for (uint8_t file = 0; file < NR_OF_ROWS; file++) {
    ZOBRIST_EP_FILE[file] = random_u64();
  }
}
