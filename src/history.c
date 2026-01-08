#include "history.h"

#include <stdlib.h>
#include <string.h>

#include "defs.h"

history_h_t hh = {{0}};

void hh_update(const move_t move, const int bonus, const board_t* board) {
  const int clamped_bonus = (bonus < -HISTORY_MAX)  ? -HISTORY_MAX
                            : (bonus > HISTORY_MAX) ? HISTORY_MAX
                                                    : bonus;
  int* entry = hh_get(move, board);
  *entry += clamped_bonus - (*entry * abs(clamped_bonus) / HISTORY_MAX);
}

int* hh_get(const move_t move, const board_t* board) {
  const square_t from = get_from(move), to = get_to(move);
  return &hh[board->side_to_move][from][to];
}

void hh_clear(void) { memset(&hh, 0, sizeof(history_h_t)); }
