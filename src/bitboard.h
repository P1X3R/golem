#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "defs.h"

FORCE_INLINE bitboard_t bit(const square_t sq) { return 1ULL << sq; }
FORCE_INLINE square_t to_square(const int rank, const int file) {
  return (rank << 3) | file;
}
FORCE_INLINE uint8_t get_rank(const square_t sq) { return sq >> 3; }
FORCE_INLINE uint8_t get_file(const square_t sq) { return sq & 7; }
FORCE_INLINE square_t pop_lsb(bitboard_t* bits) {
  const square_t sq = __builtin_ctzll(*bits);
  *bits &= *bits - 1;
  return sq;
}
FORCE_INLINE int8_t get_pawn_direction(const color_t color) {
  return (color == CLR_WHITE) ? 8 : -8;
}
FORCE_INLINE bool is_valid_coord(const int rank, const int file) {
  return rank >= 0 && rank < NR_OF_ROWS && file >= 0 && file < NR_OF_ROWS;
}
static FORCE_INLINE bitboard_t get_adjacent(const bitboard_t target) {
  return ((target & ~FH) << 1) | ((target & ~FA) >> 1);
}
FORCE_INLINE bool more_than_one(const bitboard_t target) {
  return target & (target - 1);
}
