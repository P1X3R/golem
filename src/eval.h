#pragma once

#include <stdint.h>

#include "board.h"
#include "psqt.h"

static int16_t static_eval(const board_t* board) {
  const int32_t mg_score =
      board->mg_score[CLR_WHITE] - board->mg_score[CLR_BLACK];
  const int32_t eg_score =
      board->eg_score[CLR_WHITE] - board->eg_score[CLR_BLACK];
  const uint8_t mg_phase =
      (board->phase > TOTAL_PHASE) ? TOTAL_PHASE : board->phase;
  const uint8_t eg_phase = TOTAL_PHASE - mg_phase;

  return (int16_t)(((mg_score * mg_phase) + (eg_score * eg_phase)) /
                   TOTAL_PHASE);
}
