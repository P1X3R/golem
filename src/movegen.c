#include "movegen.h"

#include <stdint.h>

#include "bitboard.h"
#include "board.h"
#include "defs.h"

static FORCE_INLINE void splat_moves(const square_t from, bitboard_t moves,
                                     const uint8_t flags,
                                     move_list_t* move_list) {
  while (moves) {
    push_move(move_list, new_move(from, pop_lsb(&moves), flags));
  }
}

static FORCE_INLINE void splat_pawn_moves(const int8_t direction,
                                          bitboard_t moves, const uint8_t flags,
                                          move_list_t* move_list) {
  while (moves) {
    const square_t to = pop_lsb(&moves);
    push_move(move_list, new_move(to + direction, to, flags));
  }
}

static FORCE_INLINE void splat_promotion_moves(const int8_t direction,
                                               bitboard_t moves,
                                               const uint8_t flags,
                                               move_list_t* move_list) {
  while (moves) {
    const square_t to = pop_lsb(&moves);
    const square_t from = to + direction;

    push_move(move_list,
              new_move(from, to, encode_promotion(PT_KNIGHT) | flags));
    push_move(move_list,
              new_move(from, to, encode_promotion(PT_BISHOP) | flags));
    push_move(move_list, new_move(from, to, encode_promotion(PT_ROOK) | flags));
    push_move(move_list,
              new_move(from, to, encode_promotion(PT_QUEEN) | flags));
  }
}

void gen_pawn_pushes(const board_t* __restrict board, const bitboard_t mask,
                     move_list_t* __restrict move_list) {
  const color_t color = board->side_to_move;
  const bitboard_t empty = ~board->occupancy;
  const bitboard_t pawns =
      board->bitboards[PT_PAWN] & board->occupancies[color];
  const bitboard_t single_pushes =
      (color == CLR_WHITE) ? (pawns << 8) & empty : (pawns >> 8) & empty;
  const int8_t down = get_pawn_direction(color ^ 1);
  bitboard_t double_pushes = (color == CLR_WHITE)
                                 ? (single_pushes << 8) & empty & R4
                                 : (single_pushes >> 8) & empty & R5;
  bitboard_t promotion_pushes =
      (color == CLR_WHITE) ? single_pushes & R8 : single_pushes & R1;
  bitboard_t quiet_pushes = single_pushes ^ promotion_pushes;

  splat_promotion_moves(down, promotion_pushes & mask, FLAG_QUIET, move_list);
  splat_pawn_moves(down, quiet_pushes & mask, FLAG_QUIET, move_list);
  splat_pawn_moves((int8_t)(down + down), double_pushes & mask,
                   FLAG_DOUBLE_PUSH, move_list);
}

void gen_pawn_captures(const board_t* __restrict board, const bitboard_t mask,
                       move_list_t* __restrict move_list) {
  const color_t color = board->side_to_move;
  const bitboard_t pawns =
      board->bitboards[PT_PAWN] & board->occupancies[color];
  const bitboard_t enemy = board->occupancies[color ^ 1];
  const bitboard_t promo_rank = (color == CLR_WHITE) ? R8 : R1;
  const int8_t push = get_pawn_direction(color ^ 1);

  const int8_t left_capture_direction = (int8_t)(push + 1);
  const bitboard_t left_captures = (color == CLR_WHITE)
                                       ? ((pawns & ~FA) << 7) & enemy
                                       : ((pawns & ~FA) >> 9) & enemy;
  bitboard_t left_captures_promo = left_captures & promo_rank;
  bitboard_t left_captures_no_promo = left_captures ^ left_captures_promo;

  const int8_t right_capture_direction = (int8_t)(push - 1);
  const bitboard_t right_captures = (color == CLR_WHITE)
                                        ? ((pawns & ~FH) << 9) & enemy
                                        : ((pawns & ~FH) >> 7) & enemy;
  bitboard_t right_captures_promo = right_captures & promo_rank;
  bitboard_t right_captures_no_promo = right_captures ^ right_captures_promo;

  splat_promotion_moves(left_capture_direction, left_captures_promo & mask,
                        FLAG_CAPTURE, move_list);
  splat_promotion_moves(right_capture_direction, right_captures_promo & mask,
                        FLAG_CAPTURE, move_list);
  splat_pawn_moves(left_capture_direction, left_captures_no_promo & mask,
                   FLAG_CAPTURE, move_list);
  splat_pawn_moves(right_capture_direction, right_captures_no_promo & mask,
                   FLAG_CAPTURE, move_list);

  if (board->ep_target == SQ_NONE) {
    return;
  }

  bitboard_t attackers = gen_piece_attacks(PT_PAWN, color ^ 1, board->occupancy,
                                           board->ep_target) &
                         pawns;
  while (attackers) {
    push_move(move_list,
              new_move(pop_lsb(&attackers), board->ep_target, FLAG_EP));
  }
}

static void gen_castling(const board_t* __restrict board,
                         move_list_t* __restrict move_list) {
  const color_t color = board->side_to_move;
  const bitboard_t occupancy = board->occupancy;
  const uint8_t rights = board->rights;

  const bitboard_t king_side_empty_mask =
      (color == CLR_WHITE) ? bit(SQ_F1) | bit(SQ_G1) : bit(SQ_F8) | bit(SQ_G8);
  const bitboard_t queen_side_empty_mask =
      (color == CLR_WHITE) ? bit(SQ_D1) | bit(SQ_C1) | bit(SQ_B1)
                           : bit(SQ_D8) | bit(SQ_C8) | bit(SQ_B8);

  const square_t to_king_side = (color == CLR_WHITE) ? SQ_G1 : SQ_G8;
  const square_t to_queen_side = (color == CLR_WHITE) ? SQ_C1 : SQ_C8;

  const uint8_t king_side_right = (color == CLR_WHITE) ? RT_WK : RT_BK;
  const uint8_t queen_side_right = (color == CLR_WHITE) ? RT_WQ : RT_BQ;

  if ((king_side_right & rights) && !(king_side_empty_mask & occupancy)) {
    push_move(move_list,
              new_move(board->kings[color], to_king_side, FLAG_KING_SIDE));
  }
  if ((queen_side_right & rights) && !(queen_side_empty_mask & occupancy)) {
    push_move(move_list,
              new_move(board->kings[color], to_queen_side, FLAG_QUEEN_SIDE));
  }
}

void gen_non_evasion_moves(const board_t* __restrict board,
                           move_list_t* __restrict move_list) {
  gen_pawn_pushes(board, UINT64_MAX, move_list);
  gen_pawn_captures(board, UINT64_MAX, move_list);

  const bitboard_t empty = ~board->occupancy;
  const bitboard_t friendly = board->occupancies[board->side_to_move];
  const bitboard_t enemy = board->occupancies[board->side_to_move ^ 1];

  for (piece_t piece = PT_KNIGHT; piece <= PT_KING; piece++) {
    bitboard_t temp = board->bitboards[piece] & friendly;

    while (temp) {
      const square_t from = pop_lsb(&temp);
      const bitboard_t attacks =
          gen_piece_attacks(piece, board->side_to_move, board->occupancy, from);

      splat_moves(from, attacks & empty, FLAG_QUIET, move_list);
      splat_moves(from, attacks & enemy, FLAG_CAPTURE, move_list);
    }
  }

  gen_castling(board, move_list);
}
