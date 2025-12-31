#include "search.h"

#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "defs.h"
#include "eval.h"
#include "misc.h"
#include "movegen.h"
#include "ordering.h"
#include "uci.h"

#define TIME_CHECK_MASK 1023

volatile _Atomic search_flag_t SEARCH_FLAG = ST_EXIT;

static FORCE_INLINE void pv_update(pv_table_t* pv, const uint8_t ply,
                                   const move_t best_move) {
  pv->table[ply][0] = best_move;

  if (ply + 1 >= MAX_PLY) {
    pv->len[ply] = 1;
    return;
  }

  const uint8_t child_len = pv->len[ply + 1];
  pv->len[ply] = child_len + 1;

  memcpy(&pv->table[ply][1], &pv->table[ply + 1][0],
         child_len * sizeof(move_t));
}

static FORCE_INLINE bool is_timeout(search_ctx_t* ctx, const bool done_depth) {
  if (search_flag_load() == ST_PONDER) {
    return false;
  }

  time_control_t* tc = &ctx->time_control;
  if (!tc->timeout && (done_depth || (ctx->nodes & TIME_CHECK_MASK) == 0)) {
    const uint64_t now = now_ms() - ctx->time_control.start_ms;
    tc->timeout = now >= tc->hard_ms || (done_depth && now >= tc->soft_ms);
  }

  return tc->timeout;
}

void* start_search(void* params) {
  assert(params != NULL);
  const uci_go_params_t p = *(uci_go_params_t*)params;

  search_ctx_t ctx = {
      .board = p.engine->board,
      .pv = (pv_table_t){{{0}}, {0}},
      .time_control = p.time_control,
      .nodes = 0,
      .seldepth = 0,
  };

  move_t ponder_move = 0;
  const move_t best_move = iterative_deepening(&ctx, &ponder_move, p.depth);

  char best_move_uci[6] = {0};
  char ponder_move_uci[6] = {0};

  move_to_uci(best_move, best_move_uci);
  if (ponder_move) {
    move_to_uci(ponder_move, ponder_move_uci);
  }

  // Engine must not send bestmove until `ponderhit` or `stop`
  while (search_flag_load() == ST_PONDER) {
    sched_yield();
  }

  printf("bestmove %s", best_move_uci);
  if (ponder_move) {
    printf(" ponder %s", ponder_move_uci);
  }
  putchar('\n');
  fflush(stdout);

  search_flag_store(ST_EXIT);

  return NULL;
}

move_t iterative_deepening(search_ctx_t* ctx, move_t* ponder_move,
                           const uint8_t depth) {
  assert(depth < MAX_PLY);

  move_t best_move = 0;

  for (uint8_t curr_depth = 1; curr_depth <= depth; curr_depth++) {
    const int score = alpha_beta(ctx, curr_depth, 0, -MATE_SCORE, MATE_SCORE);

    if (search_flag_load() == ST_EXIT || is_timeout(ctx, true)) {
      if (curr_depth <= 1) {
        send_info_depth(ctx, curr_depth, score);
        best_move = ctx->pv.table[0][0];
      }
      break;
    }

    best_move = ctx->pv.table[0][0];
    if (ctx->pv.len[0] >= 2) {
      *ponder_move = ctx->pv.table[0][1];
    }
    send_info_depth(ctx, curr_depth, score);
  }

  return best_move;
}

int alpha_beta(search_ctx_t* ctx, uint8_t depth, const uint8_t ply, int alpha,
               const int beta) {
  const bool is_root = ply == 0;
  board_t* board = &ctx->board;

  ctx->pv.len[ply] = 0;
  if (depth == 0) {
    return quiescence(ctx, ply, alpha, beta);
  }

  if (!is_root && (is_timeout(ctx, false) || search_flag_load() == ST_EXIT)) {
    return alpha;
  }
  if (search_flag_load() == ST_PONDERHIT) {
    ctx->time_control.start_ms = now_ms();
    ctx->time_control.timeout = false;
    search_flag_store(ST_THINK);
  }

  ctx->nodes++;
  ctx->seldepth = (ply > ctx->seldepth) ? ply : ctx->seldepth;

  if (!is_root && is_draw(board)) {
    return 0;
  }
  if (ply >= MAX_PLY) {
    return static_eval(board);
  }

  const bool checked = in_check(board);
  if (checked && depth < MAX_PLY - 1) {
    depth++;
  }

  move_list_t move_list = gen_color_moves(board);
  int scores[MAX_MOVES] = {0};
  score_list(ctx, &move_list, scores);

  const int max_mate = MATE_SCORE - ply;
  int max = -MATE_SCORE;
  uint8_t currmovenumber = 0;
  uint64_t last_currmove = is_root ? now_ms() : 0;

  for (uint8_t i = 0; i < move_list.len; i++) {
    next_move(&move_list, scores, i);
    const move_t move = move_list.moves[i];
    const undo_t undo = do_move(move, board);
    if (!was_legal(move, board)) {
      undo_move(undo, move, board);
      continue;
    }

    currmovenumber++;
    uint64_t now;
    if (is_root && (now = now_ms()) - last_currmove >= 1000) {
      last_currmove = now;
      send_info_currmove(move, currmovenumber);
    }

    const int score = -alpha_beta(ctx, depth - 1, ply + 1, -beta, -alpha);
    undo_move(undo, move, board);

    if (score > max) {
      max = score;
      if (score > alpha) {
        pv_update(&ctx->pv, ply, move);
        alpha = score;
      }
    }
    if (alpha >= beta) {
      break;
    }
    if (search_flag_load() == ST_EXIT || is_timeout(ctx, false)) {
      return max;
    }
  }

  if (currmovenumber) {
    return max;
  }
  if (checked) {
    return -max_mate;
  }
  return 0;
}

int quiescence(search_ctx_t* ctx, const uint8_t ply, int alpha,
               const int beta) {
  if (search_flag_load() == ST_PONDERHIT) {
    ctx->time_control.start_ms = now_ms();
    ctx->time_control.timeout = false;
    search_flag_store(ST_THINK);
  }

  ctx->nodes++;
  ctx->seldepth = (ply > ctx->seldepth) ? ply : ctx->seldepth;

  board_t* board = &ctx->board;
  int max = static_eval(board);

  if (ply >= MAX_PLY || is_timeout(ctx, false) ||
      search_flag_load() == ST_EXIT) {
    return max;
  }

  // Stand pat
  if (max >= beta) {
    return max;
  }
  if (max > alpha) {
    alpha = max;
  }

  move_list_t move_list = gen_captures_only(board);
  int scores[MAX_MOVES] = {0};
  score_list(ctx, &move_list, scores);

  for (uint8_t i = 0; i < move_list.len; i++) {
    next_move(&move_list, scores, i);
    const move_t move = move_list.moves[i];
    const undo_t undo = do_move(move, board);
    if (!was_legal(move, board)) {
      undo_move(undo, move, board);
      continue;
    }

    const int score = -quiescence(ctx, ply + 1, -beta, -alpha);
    undo_move(undo, move, board);

    if (score > max) {
      max = score;
      if (score > alpha) {
        alpha = score;
      }
    }
    if (alpha >= beta) {
      break;
    }
    if (search_flag_load() == ST_EXIT || is_timeout(ctx, false)) {
      return alpha;
    }
  }

  return max;
}
