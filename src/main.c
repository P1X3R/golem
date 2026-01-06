#include "board.h"
#include "transposition.h"
#include "uci.h"
#include "zobrist.h"

int main(void) {
  init_zobrist_tables();
  tt_init(DEFAULT_TT_SIZE);

  engine_t engine = {
      from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
  };

  uci_loop(&engine);
  return 0;
}
