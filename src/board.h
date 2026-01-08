#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "defs.h"

typedef struct {
  piece_t mailbox[NR_OF_SQUARES];
  uint64_t history[1024];
  bitboard_t bitboards[NR_OF_PIECE_TYPES];
  bitboard_t occupancies[NR_OF_COLORS];
  bitboard_t occupancy;
  uint64_t zobrist;
  int mg_score[NR_OF_COLORS];
  int eg_score[NR_OF_COLORS];
  int num_moves;
  square_t kings[NR_OF_COLORS];
  uint8_t rights;
  uint8_t halfmove_clock;
  uint8_t phase;
  square_t ep_target;
  color_t side_to_move;
} board_t;

board_t from_fen(const char fen[]);
bool was_legal(move_t move, const board_t* board);
bool in_check(const board_t* board);
undo_t do_move(move_t move, board_t* board);
void undo_move(undo_t undo, move_t move, board_t* board);
square_t do_null_move(board_t* board);
void undo_null_move(square_t ep_target, board_t* board);
bool is_draw(const board_t* board);
void print_board(const board_t* board);
