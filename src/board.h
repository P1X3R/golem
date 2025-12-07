#pragma once

#include <stddef.h>
#include <stdint.h>

#include "defs.h"

typedef struct {
  piece_t mailbox[NR_OF_SQUARES];
  bitboard_t bitboards[NR_OF_PIECE_TYPES];
  bitboard_t occupancies[NR_OF_COLORS];
  bitboard_t occupancy;
  uint64_t zobrist;
  square_t kings[NR_OF_COLORS];
  uint8_t rights;
  uint8_t halfmove_clock;
  square_t ep_target;
  color_t side_to_move;
} board_t;

bool FORCE_INLINE is_square_attakced(square_t sq, color_t attacker_color,
                                     const board_t* board,
                                     const bitboard_t occupancy);
board_t from_fen(char fen[]);
bool was_legal(move_t move, const board_t* board);
