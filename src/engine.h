#pragma once

#include "board.h"
#include "movegen.h"
#include "types.h"
#include "evaluation.h"
#include "die.h"
#include "errosion.h"

#include <atomic>
#include <vector>
#include <chrono>

namespace Leaf {

  //Transposition table
  struct TTEntry {
    Hash key;
    int depth;
    int score;
    Bound flags;
    Move best;
  };

  class TT {
    std::vector<TTEntry> TTtable;
    size_t TTmask;

    public:
    TT(size_t size = 1 << 23);
    void store (Hash key, int depth, int score, Bound flags, Move best);
    bool probe (Hash key, int depth, int& score, int alpha, int beta );
    Move getMove (Hash key);
  };

  struct PV {
    Move Table[MAX_PLY][MAX_PLY];
    //to store number of moves in best pv line....
    int length[MAX_PLY];
    
    void clear () {
      for (int i = 0; i < MAX_PLY; i++) {
        length[i] = 0;
      }
    }
  };
  struct Killer {
    Move Table[2][MAX_PLY];
    void clear () {
      for (int i = 0; i < MAX_PLY; i++) {
        Table[0][i] = Move::none();
        Table[1][i] = Move::none();
      }
    }
  };

  extern std::atomic<bool> stopSearch;

  Move FindBestMove (Board& b, int depth);
  Move SearchMove(Board& b, int maxDepth);
  int quiesciene (Board& b);
}
