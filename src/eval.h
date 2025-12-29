#pragma once

#include <stdint.h>

#include "board.h"
#include "defs.h"
#include "psqt.h"

FORCE_INLINE int static_eval(const board_t* board) {
  const int mg_score = board->mg_score[CLR_WHITE] - board->mg_score[CLR_BLACK];
  const int eg_score = board->eg_score[CLR_WHITE] - board->eg_score[CLR_BLACK];
  const uint8_t mg_phase =
      (board->phase > TOTAL_PHASE) ? TOTAL_PHASE : board->phase;
  const uint8_t eg_phase = TOTAL_PHASE - mg_phase;
  const int whites_score =
      ((mg_score * mg_phase) + (eg_score * eg_phase)) / TOTAL_PHASE;

  if (board->side_to_move == CLR_BLACK) {
    return -whites_score;
  }
  return whites_score;
}
