#include "board.h"
#include "uci.h"
#include "zobrist.h"

int main(void) {
  init_zobrist_tables();

  engine_t engine = {
      .board =
          from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
  };

  uci_loop(&engine);
  return 0;
}
