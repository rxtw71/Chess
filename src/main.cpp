#include "bitboard.h"
#include "board.h"
#include "hash.h"
#include "types.h"
#include "uci.h"


using namespace Leaf;
std::string move_to_san(Board &b, Move m);

int main() {
  Bitboards::init();
  init_hash();

  Board board;
  board.init();

  UCI_LOOP();
}
