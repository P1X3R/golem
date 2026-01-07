#include "transposition.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"

#define MB_SCALE ((size_t)1 << 20)
#define AGE_SHIFT 1

// Global transposition table
t_table_t tt = {NULL, 0, 0};

void tt_update(void) { tt.age += 1; }
void tt_prefetch(const uint64_t hash) {
  assert(tt.buckets && "TT must be initialized before search");
  __builtin_prefetch(&tt.buckets[hash & tt.mask]);
}

void tt_clear(void) {
  if (tt.buckets == NULL) {
    return;
  }

  memset(tt.buckets, 0, (tt.mask + 1) * sizeof(tt_bucket_t));
}

void tt_init(size_t mb) {
  if (tt.buckets) {
    free(tt.buckets);
    tt = (t_table_t){NULL, 0, 0};
  }

  if (mb < 2) {
    mb = 2;  // 2 MB minimum
  } else if (mb >= SIZE_MAX / MB_SCALE) {
    mb = SIZE_MAX / MB_SCALE;
  }

  const size_t bytes = mb * MB_SCALE;
  const size_t buckets = (size_t)1
                         << (8 * sizeof(size_t) - 1 -
                             __builtin_clzl(bytes / sizeof(tt_bucket_t)));

  tt.mask = buckets - 1;
  if (posix_memalign((void**)&tt.buckets, 64, buckets * sizeof(tt_bucket_t)) !=
      0) {
    tt.buckets = NULL;
    tt.mask = 0;
    return;
  }

  tt_clear();
}

uint16_t get_hashfull(void) {
  if (tt.buckets == NULL) {
    return 0;
  }

  uint16_t count = 0;

  for (uint16_t i = 0; i < 1000; i++) {
    for (uint8_t j = 0; j < BUCKETS_LEN; j++) {
      count += tt.buckets[i][j].bound != BOUND_NONE &&
               tt.buckets[i][j].age == tt.age;
    }
  }

  return count / BUCKETS_LEN;
}

tt_entry_t tt_probe(const uint64_t zobrist) {
  if (tt.buckets == NULL) {
    return (tt_entry_t){0, 0, 0, 0, BOUND_NONE, 0};
  }

  const tt_bucket_t* bucket = &tt.buckets[zobrist & tt.mask];
  for (uint8_t i = 0; i < BUCKETS_LEN; i++) {
    const tt_entry_t entry = (*bucket)[i];
    if (entry.key == zobrist && entry.bound != BOUND_NONE) {
      return entry;
    }
  }

  return (tt_entry_t){0, 0, 0, 0, BOUND_NONE, 0};
}

static FORCE_INLINE int tt_priority(const tt_entry_t* entry) {
  const uint8_t age_diff = tt.age - entry->age;
  return entry->depth - (age_diff << AGE_SHIFT);
}

void tt_store(const uint64_t zobrist, const move_t best_move, const int score,
              const uint8_t depth, const uint8_t ply, const uint8_t bound) {
  if (tt.buckets == NULL) {
    return;
  }

  tt_bucket_t* bucket = &tt.buckets[zobrist & tt.mask];
  tt_entry_t* replace = NULL;
  int replace_priority = INT32_MAX;

  for (uint8_t i = 0; i < BUCKETS_LEN; i++) {
    tt_entry_t* entry = (*bucket) + i;
    if (entry->bound == BOUND_NONE || entry->key == zobrist) {
      replace = entry;
      break;
    }
    const int entry_priority = tt_priority(entry);
    if (entry_priority < replace_priority) {
      replace = entry;
      replace_priority = entry_priority;
    }
  }

  assert(replace != NULL);
  replace->key = zobrist;
  replace->best_move = best_move;
  replace->score = encode_mate(score, ply);
  replace->depth = depth;
  replace->bound = bound;
  replace->age = tt.age;
}
