#include "board.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "defs.h"
#include "movegen.h"
#include "zobrist.h"

static void FORCE_INLINE set_piece(board_t* board, const square_t sq,
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

board_t from_fen(char fen[]) {
  board_t board = empty_board();
  char* token;

  /*
   * --- Piece placement ---
   */
  token = strtok(fen, " ");
  if (token == NULL) {
    return board;
  }

  uint8_t rank = 7, file = 0;
  for (char* current = token; *current; current++) {
    const char c = *current;

    if (c == '/') {
      rank -= 1;
      file = 0;
    } else if (c >= '1' && c <= '8') {
      file += c - '0';
    } else if (strchr("pnbrqkPNBRQK", c) != NULL) {
      set_piece(&board, to_square(rank, file++),
                char_to_piece((char)tolower((unsigned char)c)),
                isupper(c) ? CLR_WHITE : CLR_BLACK);
    }
  }

  board.occupancy = board.occupancies[CLR_WHITE] | board.occupancies[CLR_BLACK];

  /*
   * --- Side to move ---
   */
  token = strtok(NULL, " ");
  if (token == NULL) {
    return board;
  }

  switch (token[0]) {
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
  token = strtok(NULL, " ");
  if (token == NULL) {
    return board;
  }

  for (char* current = token; *current; current++) {
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
  token = strtok(NULL, " ");
  if (token == NULL) {
    return board;
  }

  if (strlen(token) >= 2) {
    char ep_file_char = token[0];
    char ep_rank_char = token[1];
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
                                 (captured_pawn & enemy_pawns) &&
                                 (get_adjacent(captured_pawn) & friendly_pawns);

      if (is_capturable) {
        board.ep_target = target;
        board.zobrist ^= ZOBRIST_EP_FILE[get_file(target)];
      }
    }
  }

  /*
   * --- Halfmove clock ---
   */
  token = strtok(NULL, " ");
  if (token == NULL) {
    return board;
  }

  char* endptr;
  board.halfmove_clock = strtoul(token, &endptr, 10);
  if (endptr == token || *endptr != '\0') {
    board.halfmove_clock = 0;
  }

  return board;
}

bool FORCE_INLINE is_square_attacked(const square_t sq,
                                     const color_t attacker_color,
                                     const board_t* board,
                                     const bitboard_t occupancy) {
  const bitboard_t attackers = board->occupancies[attacker_color];

  // Pawn attacks (reverse color direction)
  if (gen_piece_attacks(PT_PAWN, attacker_color ^ 1, occupancy, sq) &
      (board->bitboards[PT_PAWN] & attackers)) {
    return true;
  }

  // Knight attacks
  if (gen_piece_attacks(PT_KNIGHT, attacker_color, occupancy, sq) &
      (board->bitboards[PT_KNIGHT] & attackers)) {
    return true;
  }

  // Bishop / Queen attacks
  if (gen_piece_attacks(PT_BISHOP, attacker_color, occupancy, sq) &
      ((board->bitboards[PT_BISHOP] | board->bitboards[PT_QUEEN]) &
       attackers)) {
    return true;
  }

  // Rook / Queen attacks
  if (gen_piece_attacks(PT_ROOK, attacker_color, occupancy, sq) &
      ((board->bitboards[PT_ROOK] | board->bitboards[PT_QUEEN]) & attackers)) {
    return true;
  }

  // King attacks
  if (gen_piece_attacks(PT_KING, attacker_color, occupancy, sq) &
      (board->bitboards[PT_KING] & attackers)) {
    return true;
  }

  return false;
}

static const square_t CASTLE_PATHS[NR_OF_COLORS][NR_OF_CASTLING_SIDES][3] = {
    // White
    {
        {SQ_E1, SQ_F1, SQ_G1},  // King-side
        {SQ_E1, SQ_D1, SQ_C1},  // Queen-side
    },
    // Black
    {
        {SQ_E8, SQ_F8, SQ_G8},  // King-side
        {SQ_E8, SQ_D8, SQ_C8},  // Queen-side
    },
};

bool was_legal(const move_t move, const board_t* board) {
  const color_t enemy = board->side_to_move ^ 1;

  if (is_castling(move)) {
    const uint8_t flags = get_flags(move);
    const uint8_t side = (flags == FLAG_KING_SIDE) ? 0 : 1;

    for (uint8_t i = 0; i < 3; i++) {
      if (is_square_attacked(CASTLE_PATHS[board->side_to_move][side][i], enemy,
                             board, board->occupancy)) {
        return false;
      }
    }

    return true;
  }

  return !is_square_attacked(board->kings[board->side_to_move], enemy, board,
                             board->occupancy);
}
