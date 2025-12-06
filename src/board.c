#include "board.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "defs.h"
#include "zobrist.h"

static void FORCE_INLINE set_piece(board_t* __restrict board, const square_t sq,
                                   const piece_t piece, const color_t color) {
  const bitboard_t placed = bit(sq);

  board->mailbox[sq] = piece;
  board->bitboards[piece] |= placed;
  board->occupancies[color] |= placed;
  board->zobrist ^= ZOBRIST_PIECES[piece][sq];

  if (piece == PT_KING) {
    board->kings[color] = sq;
  }
}

static board_t FORCE_INLINE empty_board() {
  board_t board = {
      .mailbox = {},
      .bitboards = {},
      .occupancies = {},
      .occupancy = 0ULL,
      .zobrist = 0ULL,
      .kings = {SQ_NONE, SQ_NONE},
      .rights = 0,
      .halfmove_clock = 0,
      .ep_target = SQ_NONE,
      .side_to_move = CLR_WHITE,
  };

  for (square_t sq = SQ_A1; sq <= SQ_H8; sq++) {
    board.mailbox[sq] = PT_NONE;
  }

  return board;
}

static piece_t FORCE_INLINE char_to_piece(const char c) {
  switch (c) {
    case 'p':
      return PT_PAWN;
    case 'n':
      return PT_KNIGHT;
    case 'b':
      return PT_BISHOP;
    case 'r':
      return PT_ROOK;
    case 'q':
      return PT_QUEEN;
    case 'k':
      return PT_KING;
    default:
      return PT_NONE;
  }
}

board_t from_fen(char fen[], const size_t len) {
  board_t board = empty_board();

  /*
   * --- Piece placement ---
   */
  char* placement = strtok(fen, " ");
  square_t sq = SQ_A8;

  for (char* current = placement; *current; current++) {
    const char c = *current;

    if (c == '/') {
      sq = (sq & ~7) - 8;  // Move to A-file of next rank down
    } else if (c >= '1' && c <= '8') {
      sq += c - '0';
    } else if (strchr("pnbrqkPNBRQK", c) != NULL) {
      set_piece(&board, sq++, char_to_piece((char)tolower(c)),
                isupper(c) ? CLR_WHITE : CLR_BLACK);
    }
  }

  board.occupancy = board.occupancies[CLR_WHITE] | board.occupancies[CLR_BLACK];

  /*
   * --- Side to move ---
   */
  char* side_to_move = strtok(fen, " ");
  switch (side_to_move[0]) {
    case 'w':
      board.side_to_move = CLR_WHITE;
      break;
    case 'b':
      board.side_to_move = CLR_BLACK;
      board.zobrist ^= ZOBRIST_COLOR;
      break;
    default:
      board.side_to_move = CLR_WHITE;
      break;
  }

  /*
   * --- Castling rights ---
   */
  char* castling_rights = strtok(fen, " ");
  for (char* current = castling_rights; *current; current++) {
    switch (*current) {
      case 'K':
        board.rights |= RT_WK;
        break;
      case 'Q':
        board.rights |= RT_WQ;
        break;
      case 'k':
        board.rights |= RT_BK;
        break;
      case 'q':
        board.rights |= RT_BQ;
        break;
      default:
        break;
    }
  }
  board.zobrist ^= ZOBRIST_CASTLING_RIGHTS[board.rights];

  /*
   * --- En passant ---
   */
  char* ep_square = strtok(fen, " ");
  if (strlen(ep_square) >= 2) {
    char ep_file_char = ep_square[0];
    char ep_rank_char = ep_square[1];
    const char valid_ep_rank = (board.side_to_move == CLR_WHITE) ? '6' : '3';

    if (ep_file_char >= 'a' && ep_file_char <= 'h' &&
        ep_rank_char == valid_ep_rank) {
      const square_t target = to_square(ep_rank_char - '1', ep_file_char - 'a');
      const bitboard_t captured_pawn =
          bit(target - get_pawn_direction(board.side_to_move));
      const bitboard_t enemy_pawns =
          board.bitboards[PT_PAWN] & board.occupancies[board.side_to_move ^ 1];
      const bitboard_t friendly_pawns =
          board.bitboards[PT_PAWN] & board.occupancies[board.side_to_move];
      const bool is_capturable = board.mailbox[target] == PT_NONE &&
                                 captured_pawn & enemy_pawns &&
                                 get_adjacent(captured_pawn) & friendly_pawns;

      if (is_capturable) {
        board.ep_target = target;
        board.zobrist ^= ZOBRIST_EP_FILE[get_file(target)];
      }
    }
  }

  /*
   * --- Halfmove clock ---
   */
  char* clock = strtok(fen, " ");
  char* endptr;
  board.halfmove_clock = strtoul(clock, &endptr, 10);
  if (endptr == clock || *endptr != '\0') {
    board.halfmove_clock = 0;
  }

  return board;
}
