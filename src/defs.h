#pragma once

#include <assert.h>
#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

#define NR_OF_SQUARES 64
#define NR_OF_ROWS 8
#define NR_OF_COLORS 2
#define NR_OF_PIECE_TYPES 6

#define MAX_MOVES 255

typedef uint16_t move_t;
typedef uint8_t square_t;
typedef uint64_t bitboard_t;
typedef enum {
  PT_PAWN,
  PT_KNIGHT,
  PT_BISHOP,
  PT_ROOK,
  PT_QUEEN,
  PT_KING,
  PT_NONE,
} piece_t;
typedef enum { CLR_WHITE = 0, CLR_BLACK = 1 } color_t;
typedef struct {
  uint64_t magic;
  bitboard_t mask;
  uint32_t offset;
  uint8_t shift;
} magic_t;
typedef struct {
  move_t moves[MAX_MOVES];
  uint8_t len;
} move_list_t;

enum {
  RT_WK = 1 << 0,
  RT_WQ = 1 << 1,
  RT_BK = 1 << 2,
  RT_BQ = 1 << 3,
};
enum {
  R1 = 0x00000000000000FFULL,
  R2 = 0x000000000000FF00ULL,
  R3 = 0x0000000000FF0000ULL,
  R4 = 0x00000000FF000000ULL,
  R5 = 0x000000FF00000000ULL,
  R6 = 0x0000FF0000000000ULL,
  R7 = 0x00FF000000000000ULL,
  R8 = 0xFF00000000000000ULL,
};
enum {
  FA = 0x0101010101010101ULL << 0,
  FB = 0x0101010101010101ULL << 1,
  FC = 0x0101010101010101ULL << 2,
  FD = 0x0101010101010101ULL << 3,
  FE = 0x0101010101010101ULL << 4,
  FF = 0x0101010101010101ULL << 5,
  FG = 0x0101010101010101ULL << 6,
  FH = 0x0101010101010101ULL << 7,
};
// clang-format off
enum {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,

    SQ_NONE = 65
};
// clang-format on
enum {
  FLAG_QUIET = 0b0000,
  FLAG_DOUBLE_PUSH = 0b0001,
  FLAG_KING_SIDE = 0b0010,
  FLAG_QUEEN_SIDE = 0b0011,
  FLAG_CAPTURE = 0b0100,
  FLAG_EP = 0b0101,
  FLAG_PROMOTION = 0b1000,
};

move_t FORCE_INLINE new_move(const square_t from, const square_t to,
                             const uint8_t flags) {
  assert(from <= SQ_H8);
  assert(to <= SQ_H8);
  assert(flags <= 0xF);  // flags should be only 4-bits
  return from | (to << 6) | (flags << 12);
}
uint8_t FORCE_INLINE get_from(const move_t move) { return move & 0x3f; }
uint8_t FORCE_INLINE get_to(const move_t move) { return (move >> 6) & 0x3f; }
uint8_t FORCE_INLINE get_flags(const move_t move) { return move >> 12; }

void FORCE_INLINE push_move(move_list_t* move_list, const move_t move) {
  assert(move_list->len < MAX_MOVES);
  move_list->moves[move_list->len++] = move;
}

uint8_t FORCE_INLINE encode_promotion(const piece_t promoted) {
  return FLAG_PROMOTION | (promoted - 1);
}
piece_t FORCE_INLINE decode_promotion(const uint8_t flags) {
  return (piece_t)((0b11 & flags) + 1);
}
