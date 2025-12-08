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
static bool FORCE_INLINE is_square_attacked(square_t sq, color_t attacker_color,
                                            const board_t* board,
                                            bitboard_t occupancy);
bool was_legal(move_t move, const board_t* board);
undo_t do_move(move_t move, board_t* board);
void undo_move(undo_t* undo, board_t* board);

static const char PIECE_TO_CHAR[NR_OF_COLORS][NR_OF_PIECE_TYPES] = {
    {'P', 'N', 'B', 'R', 'Q', 'K'},  // white pieces
    {'p', 'n', 'b', 'r', 'q', 'k'},  // black pieces
};

static void print_board(const board_t* board) {
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      const square_t sq = to_square(rank, file);
      const piece_t piece = board->mailbox[sq];

      if (piece != PT_NONE) {
        const color_t color =
            (bit(sq) & board->occupancies[CLR_WHITE]) ? CLR_WHITE : CLR_BLACK;
        putchar(PIECE_TO_CHAR[color][piece]);
      } else {
        putchar('.');
      }

      putchar(' ');
    }
    putchar('\n');
  }
  putchar('\n');
}

static void print_bitboard(uint64_t bb) {
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      putchar((bb & bit(to_square(rank, file))) ? '1' : '.');
      putchar(' ');
    }
    putchar('\n');
  }
  putchar('\n');
}
