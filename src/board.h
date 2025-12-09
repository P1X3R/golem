#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bitboard.h"
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

board_t from_fen(char fen[]);
bool was_legal(move_t move, const board_t* board);
undo_t do_move(move_t move, board_t* board);
void undo_move(undo_t undo, move_t move, board_t* board);
