#pragma once

#include <stdint.h>

#include "defs.h"

extern uint64_t ZOBRIST_PIECES[NR_OF_COLORS][NR_OF_PIECE_TYPES][NR_OF_SQUARES];
extern uint64_t ZOBRIST_COLOR;
extern uint64_t ZOBRIST_CASTLING_RIGHTS[16];  // 2^4 = 16
extern uint64_t ZOBRIST_EP_FILE[NR_OF_ROWS];

void init_zobrist_tables(void);
