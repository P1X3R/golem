#pragma once

#include <stdio.h>
#include "defs.h"
#include "search.h"

#define UCI_SEND(...)    \
  do {                   \
    printf(__VA_ARGS__); \
    putchar('\n');       \
    fflush(stdout);      \
  } while (0)

FORCE_INLINE void square_to_uci(const square_t sq, char out[2]) {
  out[0] = (char)('a' + (sq & 7));
  out[1] = (char)('1' + (sq >> 3));
}

FORCE_INLINE char promo_to_char(const piece_t piece) {
  switch (piece) {
    case PT_KNIGHT:
      return 'n';
    case PT_BISHOP:
      return 'b';
    case PT_ROOK:
      return 'r';
    case PT_QUEEN:
      return 'q';
    default:
      return 'q';  // Fallback, should not happen
  }
}

FORCE_INLINE void move_to_uci(const move_t move, char out[6]) {
  if (move == 0) {
    out = "0000\0\0";
  }

  square_t from = get_from(move);
  square_t to = get_to(move);
  uint8_t flags = get_flags(move);

  square_to_uci(from, out);
  square_to_uci(to, out + 2);

  uint8_t len = 4;

  if (flags & FLAG_PROMOTION) {
    const piece_t promo = decode_promotion(flags);
    out[4] = promo_to_char(promo);
    len = 5;
  }

  out[len] = '\0';
}

void uci_loop(engine_t* engine);
void send_info_depth(search_ctx_t* ctx, uint8_t depth, int score);
void send_info_currmove(move_t move, uint8_t currmovenumber);
