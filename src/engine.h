#pragma once

#include "board.h"
#include "movegen.h"
#include "types.h"
#include "evaluation.h"

#include <vector>
#include <chrono>

namespace Engine {

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

  bool DrawGame (Board& b);
  Move FindBestMove (Board& b, int depth);
  Move SearchMove(Board& b, int maxDepth, int timeMs);
  int quiesciene (Board& b);
}
