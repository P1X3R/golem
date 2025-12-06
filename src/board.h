#pragma once

#include <stdint.h>

#include "defs.h"

typedef struct {
  piece_t mailbox[NR_OF_SQUARES];
  bitboard_t bitboards[NR_OF_PIECE_TYPES][NR_OF_COLORS];
  bitboard_t occupancies[NR_OF_COLORS];
  bitboard_t occupancy;
  uint64_t zobrist;
  square_t kings[NR_OF_COLORS];
  uint8_t rights;
  uint8_t halfmove_clock;
  square_t en_passant_square;
  color_t side_to_move;
} board_t;
