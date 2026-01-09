#include "search.h"

#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "defs.h"
#include "eval.h"
#include "history.h"
#include "misc.h"
#include "movegen.h"
#include "ordering.h"
#include "transposition.h"
#include "uci.h"

#define TIME_CHECK_MASK 1023

volatile _Atomic search_flag_t SEARCH_FLAG = ST_EXIT;

static FORCE_INLINE void check_ponderhit(search_ctx_t* ctx) {
  if (search_flag_load() == ST_PONDERHIT) {
    ctx->time_control.start_ms = now_ms();
    ctx->time_control.timeout = false;
    search_flag_store(ST_THINK);
  }
}

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
      .killers = {{0}},
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

  tt_update();
  search_flag_store(ST_EXIT);

  return NULL;
}

static void update_heuristics(search_ctx_t* __restrict ctx, const uint8_t ply,
                              const uint8_t depth, const move_t move,
                              const uint8_t idx,
                              const move_list_t* __restrict move_list,
                              const move_t hash_move) {
  if (ctx->killers[ply][0] != move) {
    ctx->killers[ply][1] = ctx->killers[ply][0];
    ctx->killers[ply][0] = move;
  }

  const int bonus = depth * depth;
  hh_update(move, bonus, &ctx->board);

  // Apply history maluses
  for (uint8_t i = 0; i < idx; i++) {
    const move_t quiet_move = move_list->moves[i];
    if (is_quiet(quiet_move) && quiet_move != ctx->killers[ply][1] &&
        quiet_move != hash_move) {
      hh_update(quiet_move, -bonus, &ctx->board);
    }
  }
}

move_t iterative_deepening(search_ctx_t* ctx, move_t* ponder_move,
                           const uint8_t depth) {
  move_t best_move = 0;

  for (uint8_t curr_depth = 1; curr_depth <= depth; curr_depth++) {
    const int score = alpha_beta(ctx, curr_depth, 0, -MATE_SCORE, MATE_SCORE);

    if (search_flag_load() == ST_EXIT || is_timeout(ctx, true)) {
      if (curr_depth <= 1) {
        best_move = ctx->pv.table[0][0];
        send_info_depth(ctx, curr_depth, score);
      }
      break;
    }

    best_move = ctx->pv.table[0][0];
    *ponder_move = 0;
    if (ctx->pv.len[0] >= 2) {
      *ponder_move = ctx->pv.table[0][1];
    }
    send_info_depth(ctx, curr_depth, score);
  }

  return best_move;
}

int alpha_beta(search_ctx_t* ctx, uint8_t depth, const uint8_t ply, int alpha,
               const int beta) {
  ctx->pv.len[ply] = 0;
  if (depth == 0) {
    return quiescence(ctx, ply, alpha, beta);
  }

  const bool is_pv = alpha != beta - 1;
  const bool is_root = ply == 0;
  board_t* board = &ctx->board;

  check_ponderhit(ctx);

  ctx->nodes++;
  ctx->seldepth = (ply > ctx->seldepth) ? ply : ctx->seldepth;

  tt_prefetch(board->zobrist);
  const tt_entry_t tt_entry = tt_probe(board->zobrist);
  int tt_score = -MATE_SCORE;

  if (!is_root) {
    if (is_draw(board)) {
      return 0;
    }

    if (tt_entry.bound != BOUND_NONE && tt_entry.depth >= depth) {
      tt_score = decode_mate(tt_entry.score, ply);
      if (tt_entry.bound == BOUND_EXACT ||
          (!is_pv && tt_entry.bound == BOUND_LOWER && tt_score >= beta) ||
          (!is_pv && tt_entry.bound == BOUND_UPPER && tt_score <= alpha)) {
        return tt_score;
      }
    }

    if (ply >= MAX_PLY || is_timeout(ctx, false) ||
        search_flag_load() == ST_EXIT) {
      return static_eval(board);
    }
  }

  const bool checked = in_check(board);
  if (checked && depth < MAX_PLY - 1) {
    depth++;
  }

  // Null move pruning
  const bitboard_t non_pawn_material =
      board->occupancies[board->side_to_move] &
      ~(board->bitboards[PT_PAWN] | board->bitboards[PT_KING]);
  if (depth >= 3 && !is_root && !is_pv && !checked &&
      (tt_entry.bound == BOUND_NONE || tt_entry.bound != BOUND_UPPER ||
       tt_score >= beta) &&
      non_pawn_material) {
    const square_t ep_target = do_null_move(board);

    const uint8_t R = depth > 6 ? 4 : 3;
    int next_depth = (int)depth - 1 - R;
    if (next_depth < 0) {
      next_depth = 0;
    }

    const int score = -alpha_beta(ctx, next_depth, ply + 1, -beta, -beta + 1);

    undo_null_move(ep_target, board);

    // Don't return mates
    if (score >= beta) {
      return (score >= MATE_THRESHOLD) ? beta : score;
    }
  }

  const int alpha_original = alpha;
  move_list_t move_list = gen_color_moves(board);
  int scores[MAX_MOVES] = {0};
  score_list(ctx, &move_list, &tt_entry, ply, scores);

  const int max_mate = MATE_SCORE - ply;
  bool found_pv = false;
  move_t best_move = 0;
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

    int score;
    if (!found_pv) {
      score = -alpha_beta(ctx, depth - 1, ply + 1, -beta, -alpha);
    } else {
      score = -alpha_beta(ctx, depth - 1, ply + 1, -alpha - 1, -alpha);
      if (score > alpha && score < beta) {
        score = -alpha_beta(ctx, depth - 1, ply + 1, -beta, -alpha);
      }
    }

    undo_move(undo, move, board);

    if (score > max) {
      max = score;
      best_move = move;
      if (score > alpha) {
        pv_update(&ctx->pv, ply, move);
        found_pv = true;
        alpha = score;
      }
    }
    if (alpha >= beta) {
      if (is_quiet(move)) {
        update_heuristics(ctx, ply, depth, move, i, &move_list,
                          tt_entry.best_move);
      }
      break;
    }
    if (search_flag_load() == ST_EXIT || is_timeout(ctx, false)) {
      return max;
    }
  }

  if (currmovenumber) {
    uint8_t bound;
    if (max <= alpha_original) {
      bound = BOUND_UPPER;
    } else if (max >= beta) {
      bound = BOUND_LOWER;
    } else {
      bound = BOUND_EXACT;
    }

    tt_store(board->zobrist, best_move, max, depth, ply, bound);

    return max;
  }

  if (checked) {
    return -max_mate;
  } else {
    return 0;
  }
}

int quiescence(search_ctx_t* ctx, const uint8_t ply, int alpha,
               const int beta) {
  check_ponderhit(ctx);

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
  score_list(ctx, &move_list, NULL, ply, scores);

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
      return max;
    }
  }

  return max;
}
