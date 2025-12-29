#pragma once

#include <stdatomic.h>
#include <stdint.h>

#include "board.h"
#include "defs.h"

#define MAX_PLY 128
#define MATE_SCORE 30000
#define MATE_THRESHOLD (MATE_SCORE - (MAX_PLY * 2))

typedef struct {
  move_t table[MAX_PLY][MAX_PLY];
  uint8_t len[MAX_PLY];
} pv_table_t;

typedef struct {
  bool timeout;
  uint64_t start_ms;
  uint64_t soft_ms, hard_ms;
} time_control_t;

typedef struct {
  board_t board;
  pv_table_t pv;
  time_control_t time_control;
  uint64_t nodes;
  uint8_t seldepth;
} search_ctx_t;

typedef struct {
  board_t board;
} engine_t;

typedef enum {
  ST_THINK,
  ST_PONDER,
  ST_PONDERHIT,
  ST_EXIT,
} search_flag_t;

typedef struct {
  engine_t* engine;
  time_control_t time_control;
  uint8_t depth;
} uci_go_params_t;

extern volatile _Atomic search_flag_t SEARCH_FLAG;

FORCE_INLINE search_flag_t search_flag_load(void) {
  return atomic_load_explicit(&SEARCH_FLAG, memory_order_acquire);
}

FORCE_INLINE void search_flag_store(const search_flag_t value) {
  atomic_store_explicit(&SEARCH_FLAG, value, memory_order_release);
}

void* start_search(void* params);
move_t iterative_deepening(search_ctx_t* ctx, move_t* ponder_move,
                           uint8_t depth);
int alpha_beta(search_ctx_t* ctx, uint8_t depth, uint8_t ply, int alpha,
               int beta);
