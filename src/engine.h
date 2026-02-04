#pragma once

#include "board.h"
#include "movegen.h"
#include "types.h"
#include "evaluation.h"
#include "die.h"
#include "errosion.h"

#include <atomic>
#include <functional>
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
    TT(size_t size = 1 << 23) {
      TTtable.resize(size);
      TTmask = size - 1;
    }
    void store (Hash key, int depth, int score, Bound flags, Move best);
    bool probe (Hash key, int depth, int& score, int alpha, int beta, Bound& flag);
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
  int quiesciene(Board &b, int alpha, int beta);
  void sortMoveList (Board& b, MoveList& list, Move& tt, Move& pv, Move& k1, Move& k2);
  std::vector <Move> readPV ();
}

namespace Leaf {
  inline void TT::store (Hash key, int depth, int score, Bound flags, Move best) {
    TTEntry& e = TTtable[key & TTmask];

    if (e.depth <= depth || e.key == 0) {
      e.key = key;
      e.depth = depth;
      e.score = score;
      e.flags = flags;
      e.best = best;
    }
  }

  inline bool TT::probe (Hash key, int depth, int& score, int alpha, int beta, Bound& flag) {
    TTEntry& e = TTtable[key & TTmask];

    if (e.key != key)
      return false;
    if (e.depth < depth)
      return false;
    score = e.score;
    flag = e.flags;

    switch (e.flags) {
      case BOUND_EXACT : return true;
      case BOUND_LOWER : if (score >= beta) return true; break;
      case BOUND_UPPER : if (score <= alpha) return true; break;
      default: break;
    }
    return false;
  }

  inline Move TT::getMove (Hash key) {
    TTEntry& e = TTtable[key & TTmask];

    if (key == e.key)
      return e.best;
    return Move::none();
  }

}
