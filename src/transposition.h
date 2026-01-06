#pragma once

#include <stdalign.h>
#include <stdint.h>

#include "defs.h"
#include "search.h"

#define DEFAULT_TT_SIZE 32

enum {
  BOUND_NONE,
  BOUND_EXACT,
  BOUND_UPPER,
  BOUND_LOWER,

  BUCKETS_LEN = 4
};

typedef struct {
  uint64_t key;
  move_t best_move;
  int16_t score;
  uint8_t depth;
  uint8_t bound;
  uint8_t age;
} tt_entry_t;

typedef tt_entry_t tt_bucket_t[BUCKETS_LEN];

typedef struct {
  tt_bucket_t* buckets;
  uint64_t mask;
  uint8_t age;
} t_table_t;

static FORCE_INLINE int16_t encode_mate(const int score, const int ply) {
  if (score >= MATE_THRESHOLD) {
    return (int16_t)(score + ply);
  }
  if (score <= -MATE_THRESHOLD) {
    return (int16_t)(score - ply);
  }
  return (int16_t)score;
}

static FORCE_INLINE int decode_mate(const int16_t score, const int ply) {
  if (score >= MATE_THRESHOLD) {
    return score - ply;
  }
  if (score <= -MATE_THRESHOLD) {
    return score + ply;
  }
  return score;
}

void tt_update(void);
void tt_prefetch(uint64_t hash);

void tt_clear(void);
void tt_init(size_t mb);
uint16_t get_hashfull(void);

tt_entry_t tt_probe(uint64_t zobrist);
void tt_store(uint64_t zobrist, move_t best_move, int score, uint8_t depth,
              uint8_t ply, uint8_t bound);
