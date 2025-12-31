#pragma once

#include <stdint.h>

#include "defs.h"

#define TOTAL_PHASE 24  // Phase at starting position

typedef struct {
  int mg;
  int eg;
} score_t;

extern const score_t PSQT[NR_OF_PIECE_TYPES][NR_OF_SQUARES];
extern const uint8_t PHASE_VALUES[NR_OF_PIECE_TYPES];
