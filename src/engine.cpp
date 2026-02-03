#include "engine.h"
#include "board.h"
#include "evaluation.h"
#include "movegen.h"
#include "types.h"
#include <chrono>
#include <ctime>

namespace Leaf {

  //time controls
  using Clock = std::chrono::steady_clock;
  Clock::time_point startSearch;
  int timeMs;
  std::atomic <bool> stopSearch(false);

  inline bool timeUp () {
    if (timeMs <= 0)
      return false;
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startSearch).count() >= timeMs;
  }


  int NegaMax (Board& b, int depth, int ply) {
    if (stopSearch.load(std::memory_order_relaxed) || timeUp())
      return 0;
    //Checkmate and stalemate conditions
    if (DrawGame(b))
      return VALUE_DRAW;

    MoveList list;
    LegalMoves(b, list);
    if (list.count == 0) {
      if (b.kingInCheck(b.turn))
        return -VALUE_MATE + ply;
      else return VALUE_DRAW;
    }

    if (depth == 0)
      return Eval(b);

    int score;
    int bestScore = -VALUE_INFINITE;
    Move bestMove = Move::none();

    for (int i = 0; i < list.count; i++) {

      Board::State state;
      b.MakeMove(list.data[i], state);
      score = -NegaMax(b, depth - 1, ply + 1);
      b.UnmakeMove(list.data[i]);
     
      if (stopSearch.load(std::memory_order_relaxed) || timeUp())
        return 0;

      if (score > bestScore) {
        bestScore = score;
        bestMove = list.data[i];
      }

    }
    return bestScore;
  }

  Move FindBestMove (Board& b, int depth) {
    
    Move bestMove = Move::none();
    if (stopSearch.load(std::memory_order_relaxed) || timeUp())
      return bestMove;

    int bestScore = -VALUE_INFINITE;

    MoveList list;
    LegalMoves(b, list);
    Move none = Move::none();


    int score;
    for (int i = 0; i < list.count; i++) {

      Board::State state;
      b.MakeMove(list.data[i], state);
      score = -NegaMax(b, depth - 1, 1);
      b.UnmakeMove(list.data[i]);

      if (stopSearch.load(std::memory_order_relaxed) || timeUp())
        return bestMove;

      if (score > bestScore) {
        bestScore = score;
        bestMove = list.data[i];
      }
    }

    return bestMove;
  }


  Move SearchMove (Board& b, int maxDepth) {
    stopSearch.store(false);
    startSearch = Clock::now();

    Move none = Move::none();
    Move bestMove = Move::none();

    for (int depth = 1; depth <= maxDepth; depth++) {

      Move m = FindBestMove(b, depth);

      if (stopSearch.load(std::memory_order_relaxed) || timeUp())
        return bestMove;
      bestMove = m;
    }

    return bestMove;
  }
}
