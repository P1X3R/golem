#pragma once

#include "board.h"
#include "defs.h"

#define HISTORY_MAX 8192

typedef int history_h_t[NR_OF_COLORS][NR_OF_SQUARES][NR_OF_SQUARES];

extern history_h_t hh;

void hh_update(move_t move, int bonus, const board_t* board);
int* hh_get(move_t move, const board_t* board);
void hh_clear(void);
