#include <stdint.h>

#include "defs.h"
#include "search.h"
#include "transposition.h"

static const int PIECE_SCORE[NR_OF_PIECE_TYPES + 1] = {
    100, 300, 325, 500, 900, 0, 0,
};
static const int MVV_LVA[NR_OF_PIECE_TYPES][NR_OF_PIECE_TYPES] = {
    {109, 107, 107, 105, 101, 0}, {309, 307, 307, 305, 301, 0},
    {334, 332, 332, 330, 326, 0}, {509, 507, 507, 505, 501, 0},
    {909, 907, 907, 905, 901, 0}, {0, 0, 0, 0, 0, 0},
};

static FORCE_INLINE int score_move(const move_t move,
                                   const search_ctx_t* __restrict ctx,
                                   const tt_entry_t* __restrict entry) {
  if (entry != NULL && entry->best_move == move) {
    return 10000;
  }

  const uint8_t flags = get_flags(move);

  if (flags & FLAG_CAPTURE) {
    const square_t from = get_from(move), to = get_to(move);
    const piece_t promotion =
        (flags & FLAG_PROMOTION) ? decode_promotion(flags) : PT_NONE;
    const piece_t victim =
                      (flags == FLAG_EP) ? PT_PAWN : ctx->board.mailbox[to],
                  attacker = ctx->board.mailbox[from];

    return MVV_LVA[victim][attacker] + PIECE_SCORE[promotion];
  }

  return 0;
}

void score_list(const search_ctx_t* __restrict ctx,
                const move_list_t* __restrict move_list,
                const tt_entry_t* __restrict entry, int scores[MAX_MOVES]) {
  for (uint8_t i = 0; i < move_list->len; i++) {
    scores[i] = score_move(move_list->moves[i], ctx, entry);
  }
}

void next_move(move_list_t* move_list, int scores[MAX_MOVES],
               uint8_t start_idx) {
  int best_score = scores[start_idx];
  uint8_t best_idx = start_idx;

  // Find the best move from start_idx to end
  for (uint8_t i = start_idx + 1; i < move_list->len; i++) {
    if (scores[i] > best_score) {
      best_score = scores[i];
      best_idx = i;
    }
  }

  // Swap the best move to start_idx
  if (best_idx != start_idx) {
    move_t tmp_move = move_list->moves[start_idx];
    move_list->moves[start_idx] = move_list->moves[best_idx];
    move_list->moves[best_idx] = tmp_move;

    int tmp_score = scores[start_idx];
    scores[start_idx] = scores[best_idx];
    scores[best_idx] = tmp_score;
  }
}
