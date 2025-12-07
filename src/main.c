#include <stdint.h>
#include <stdio.h>

#include "bitboard.h"
#include "board.h"
#include "defs.h"
#include "movegen.h"
#include "zobrist.h"

const char PIECE_TO_CHAR[NR_OF_COLORS][NR_OF_PIECE_TYPES] = {
    {'P', 'N', 'B', 'R', 'Q', 'K'},  // white pieces
    {'p', 'n', 'b', 'r', 'q', 'k'},  // black pieces
};

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

void print_board(const board_t* board) {
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

void square_to_string(const square_t sq, char* str) {
  if (sq == SQ_NONE) {
    str[0] = '-';
    str[1] = '-';
    return;
  }

  str[0] = (char)('a' + get_file(sq));
  str[1] = (char)('1' + get_rank(sq));
}

void move_to_uci(const move_t move, char* str) {
  const uint8_t flags = get_flags(move);

  square_to_string(get_from(move), str);
  square_to_string(get_to(move), str + 2);
  if (flags & FLAG_PROMOTION) {
    str[4] = PIECE_TO_CHAR[1][decode_promotion(flags)];
  }
}

int main(int argc, char* argv[]) {
  init_zobrist_tables();
  char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  const board_t board = from_fen(fen);

  char ep_target[3] = "\0\0\0";
  square_to_string(board.ep_target, ep_target);

  move_list_t move_list = {{}, 0};
  gen_non_evasion_moves(&board, &move_list);

  print_board(&board);
  printf(
      "side to move: %s\ncastling rights: %c%c%c%c\nen passant target: %s\n"
      "half-move clock: %d\n",
      (board.side_to_move == CLR_WHITE) ? "white" : "black",
      (board.rights & RT_WK) ? 'K' : '-', (board.rights & RT_WQ) ? 'Q' : '-',
      (board.rights & RT_BK) ? 'k' : '-', (board.rights & RT_BQ) ? 'q' : '-',
      ep_target, board.halfmove_clock);

  printf("moves: ");
  for (uint8_t i = 0; i < move_list.len; i++) {
    const move_t move = move_list.moves[i];
    char move_str[5] = "\0\0\0\0\0";
    move_to_uci(move, move_str);

    printf("%s ", move_str);
  }
  printf("\n");
}
