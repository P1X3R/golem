#pragma once

#include "defs.h"
#include "search.h"
#include "transposition.h"

void score_list(const search_ctx_t* __restrict ctx,
                const move_list_t* __restrict move_list,
                const tt_entry_t* __restrict entry, int scores[MAX_MOVES]);
void next_move(move_list_t* move_list, int scores[MAX_MOVES],
               uint8_t start_idx);
