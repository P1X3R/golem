#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "defs.h"

bitboard_t FORCE_INLINE bit(const square_t sq) { return 1ULL << sq; }
square_t FORCE_INLINE to_square(const int rank, const int file) {
  return (rank << 3) | file;
}
uint8_t FORCE_INLINE get_rank(const square_t sq) { return sq >> 3; }
uint8_t FORCE_INLINE get_file(const square_t sq) { return sq & 7; }
square_t FORCE_INLINE pop_lsb(bitboard_t* __restrict bits) {
  const square_t sq = __builtin_ctzll(*bits);
  *bits &= *bits - 1;
  return sq;
}
int8_t FORCE_INLINE get_pawn_direction(const color_t color) {
  return (color == WHITE) ? 8 : -8;
}
bool FORCE_INLINE is_valid_coord(const int rank, const int file) {
  return rank >= 0 && rank < NR_OF_ROWS && file >= 0 && file < NR_OF_ROWS;
}
