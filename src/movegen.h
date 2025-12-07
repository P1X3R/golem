#pragma once

#include <assert.h>

#include "board.h"
#include "defs.h"
#include "luts.h"

uint32_t FORCE_INLINE get_magic_index(const magic_t* entry,
                                      const bitboard_t occupancy) {
  return entry->offset +
         (((entry->mask & occupancy) * entry->magic) >> entry->shift);
}

bitboard_t FORCE_INLINE gen_piece_attacks(const piece_t piece,
                                          const color_t color,
                                          const bitboard_t occupancy,
                                          const square_t sq) {
  assert(piece != PT_NONE);
  switch (piece) {
    case PT_PAWN:
      return (color == CLR_WHITE) ? WPAWN_ATTACKS_LUT[sq]
                                  : BPAWN_ATTACKS_LUT[sq];
    case PT_KNIGHT:
      return KNIGHT_ATTACKS_LUT[sq];
    case PT_BISHOP:
      return SLIDING_ATTACKS_LUT[get_magic_index(&BISHOP_MAGICS[sq],
                                                 occupancy)];
    case PT_ROOK:
      return SLIDING_ATTACKS_LUT[get_magic_index(&ROOK_MAGICS[sq], occupancy)];
    case PT_QUEEN:
      return SLIDING_ATTACKS_LUT[get_magic_index(&BISHOP_MAGICS[sq],
                                                 occupancy)] |
             SLIDING_ATTACKS_LUT[get_magic_index(&ROOK_MAGICS[sq], occupancy)];
    case PT_KING:
      return KING_ATTACKS_LUT[sq];
    case PT_NONE:
      return 0ULL;
  }
}

void gen_non_evasion_moves(const board_t* __restrict board,
                           move_list_t* __restrict move_list);
