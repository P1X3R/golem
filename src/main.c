#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "defs.h"
#include "movegen.h"
#include "zobrist.h"

void square_to_string(const square_t sq, char* str) {
  if (sq == SQ_NONE) {
    str[0] = '-';
    str[1] = '-';
    return;
  }

  str[0] = (char)('a' + get_file(sq));
  str[1] = (char)('1' + get_rank(sq));
}

static const char PIECE_TO_CHAR[NR_OF_PIECE_TYPES] = {
    'p', 'n', 'b', 'r', 'q', 'k',
};

void move_to_uci(const move_t move, char* str) {
  const uint8_t flags = get_flags(move);

  square_to_string(get_from(move), str);
  square_to_string(get_to(move), str + 2);
  if (flags & FLAG_PROMOTION) {
    str[4] = PIECE_TO_CHAR[decode_promotion(flags)];
  }
}

uint64_t perft(board_t* board, const uint8_t depth, const bool divide) {
  if (depth == 0) {
    return 1;
  }

  uint64_t nodes = 0;

  move_list_t move_list = {{0}, 0};
  gen_non_evasion_moves(board, &move_list);

  for (uint8_t i = 0; i < move_list.len; i++) {
    const move_t move = move_list.moves[i];

    const undo_t undo = do_move(move, board);
    if (was_legal(move, board)) {
      const uint64_t subnodes = perft(board, depth - 1, false);
      nodes += subnodes;

      if (divide) {
        char move_str[6] = {0};
        move_to_uci(move, move_str);
        printf("%s %lu\n", move_str, subnodes);
      }
    }
    undo_move(undo, move, board);
  }

  return nodes;
}

int main(int argc, char* argv[]) {
  init_zobrist_tables();

  if (argc < 3) {
    goto usage;
  }

  // Parse depth
  char* endptr;
  uint8_t depth = (uint8_t)strtoul(argv[1], &endptr, 10);
  if (endptr == argv[1] || *endptr != '\0') {
    goto usage;
  }

  // Parse FEN
  board_t board = from_fen(argv[2]);

  // Parse moves
  if (argc >= 4) {
    char tmp[256];
    strncpy(tmp, argv[3], sizeof(tmp) - sizeof(*tmp));
    tmp[sizeof(tmp) - 1] = '\0';
    char* token = strtok(tmp, " ");

    while (token != NULL) {
      if (strlen(token) < 4) {
        continue;
      }

      const square_t from = to_square(token[1] - '1', token[0] - 'a');
      const square_t to = to_square(token[3] - '1', token[2] - 'a');

      move_list_t move_list = {{0}, 0};
      gen_non_evasion_moves(&board, &move_list);

      for (uint8_t i = 0; i < move_list.len; i++) {
        const move_t move = move_list.moves[i];

        if (get_from(move) == from && get_to(move) == to) {
          do_move(move, &board);
          break;
        }
      }

      token = strtok(NULL, " ");
    }
  }

  printf("\n%lu\n", perft(&board, depth, true));
  return 0;

usage:
  puts("usage: golem <depth> <fen> {moves}");
  return 0;
}
