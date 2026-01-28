#include "engine.h"
#include "board.h"
#include "evaluation.h"
#include "movegen.h"
#include "types.h"
#include <algorithm>
#include <chrono>

namespace Engine {

  using Clock = std::chrono::steady_clock;
  Clock::time_point searchStart;
  int searchTimeLimitMs;
  bool stopSearch = false;

  inline bool timeup() {
    auto now = Clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStart).count();
    return elapsed >= searchTimeLimitMs;
  }

  //load transpoition tabl;e fucntions
  TT::TT(size_t size) {
    TTtable.resize(size);
    TTmask = size - 1;
  }
  void TT::store (Hash key, int depth, int score, Bound flags, Move best) {
    size_t index = key & TTmask;
    TTtable[index] = {key, depth, score, flags, best};
  }
  bool TT::probe (Hash key, int depth, int& score, int alpha, int beta) {
    size_t index = key & TTmask;
    auto& entry = TTtable[index];

    if (entry.key != key) return false;
    if (entry.depth < depth) return false;

    switch (entry.flags) {
      case BOUND_EXACT : score = entry.score; return true;
      case BOUND_LOWER : alpha = std::max(alpha, entry.score); break;
      case BOUND_UPPER : beta = std::min(beta, entry.score); break;
      default : return false;
    }

    if (alpha > beta) {
      score = entry.score;
      return true;
    }

    return false;
  }
  Move TT::getMove (Hash key) {
    size_t index = key & TTmask;
    auto& entry = TTtable[index];

    if (entry.key == key)
      return entry.best;
    return Move::none();
  }



  bool insufficientMaterial (Board& b) {
    Bitboard pawns = b.piecebb[W_PAWN] | b.piecebb[B_PAWN];
    Bitboard queens = b.piecebb[W_QUEEN] | b.piecebb[B_QUEEN];
    Bitboard rooks = b.piecebb[W_ROOK] |b.piecebb[B_ROOK];

    if ((pawns || queens || rooks) != 0)
      return false;

    int whiteBishops = popcount(b.piecebb[W_BISHOP]);
    int blackBishops = popcount(b.piecebb[B_BISHOP]);
    int whiteKnights = popcount(b.piecebb[W_KNIGHT]);
    int blackKnights = popcount(b.piecebb[B_KNIGHT]);
    int whiteminors = whiteBishops + whiteKnights;
    int blackminors = blackBishops + blackKnights;

    if (whiteminors == 0 && blackminors == 0)
      return true;
    //KN VS K || KB VS K
    if (whiteminors == 1 && blackminors == 0)
      return true;
    //K VS KN || K VS KB
    if (whiteminors == 0 && blackminors == 1)
      return true;
    if (whiteminors == 1 && blackminors == 1)
      return true;

    return false;
  }

  bool DrawGame (Board& b) {
    //50 move rule
    if (b.halfMove == 50)
      return true;

    //insufficient material
    if (insufficientMaterial(b))
      return true;

    //threefold repitation;
    int count = 0;
    Hash currentkey = b.key;
    for (auto it = b.history.rbegin(); it != b.history.rend(); ++it) {
      if (it->key == currentkey)
        count++;

      if (count == 3)
        return true;

      if (it->halfMove == 0)
        break;
    }
    
    return false;
  }



  TT transpositionTable;

  int quiesciene(Board& b, int alpha, int beta) {
    int static_eval = Eval(b);

    if (static_eval >= beta)
      return beta;
    if (static_eval > alpha)
      alpha = static_eval;

    MoveList list;
    LegalMoves(b, list);

    MoveList qs;
    for (int i = 0; i < list.count; i++) {
      Move m = list.data[i];
      if (b.isCapture(m) || b.givesCheck(m) || m.type_of() == PROMOTION) {
        qs.add(m);
      }
    }

    Board::State state;
    for (int i = 0; i < qs.count; i++) {
      b.MakeMove(qs.data[i], state);
      int score = -quiesciene(b, -beta, -alpha);
      b.UnmakeMove(qs.data[i]);
    
        if (score >= beta)
        return beta;
      if (score > alpha)
        alpha = score;
    }

    return alpha;
  }

  int NegaMax (Board& b, int depth, int ply, int alpha, int beta) {

    if (timeup())
      stopSearch = true;

    if (stopSearch)
      return 0;

    int TTscore;
    if (transpositionTable.probe(b.key, depth, TTscore, alpha, beta))
      return TTscore;

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
      return quiesciene(b, alpha, beta);
      //return Eval(b);

    Move TTmove = transpositionTable.getMove(b.key);
    sortMoveList(b, list, TTmove);


    int score;
    int alphaori = alpha;
    int alphaOrig = alpha;
    int bestScore = -VALUE_INFINITE;
    Move bestMove = Move::none();

    for (int i = 0; i < list.count; i++) {

      Board::State state;
      b.MakeMove(list.data[i], state);
      score = -NegaMax(b, depth - 1, ply + 1, -beta, -alpha);
      b.UnmakeMove(list.data[i]);

      if (score > bestScore) {
        bestScore = score;
        bestMove = list.data[i];
      }

      if (score >= beta) {
        transpositionTable.store(b.key, depth, bestScore, BOUND_LOWER, bestMove);
        return beta;
      }

      if (score > alpha)
        alpha = score;

    }

    Bound flag;
    if (score <= alphaOrig) flag = BOUND_UPPER;
    else if (score >= beta) flag = BOUND_LOWER;
    else flag = BOUND_EXACT;

    transpositionTable.store(b.key, depth, bestScore, flag, bestMove);
    return bestScore;
  }

  Move FindBestMove (Board& b, int depth) {
    Move bestMove = Move::none();
    int bestScore = -VALUE_INFINITE;

    MoveList list;
    LegalMoves(b, list);
    Move none = Move::none();
    sortMoveList(b, list, none);
    int score;

    for (int i = 0; i < list.count; i++) {

      if (timeup())
        stopSearch = true;

      if (stopSearch)
        break;

      Board::State state;
      b.MakeMove(list.data[i], state);
      score = -NegaMax(b, depth - 1, 1, -VALUE_INFINITE, VALUE_INFINITE);
      b.UnmakeMove(list.data[i]);

      if (stopSearch)
        break;

      if (score > bestScore) {
        bestScore = score;
        bestMove = list.data[i];
      }
    }

    if (!stopSearch)
      return bestMove;

    return bestMove;
  }

  Move SearchMove (Board& b, int maxDepth, int timeMs) {
    searchStart = Clock::now();
    searchTimeLimitMs = timeMs;

    Move none = Move::none();
    Move bestMove = Move::none();

    for (int depth = 1; depth < maxDepth; depth++) {
      stopSearch = false;

      Move m = FindBestMove(b, depth);

      if (stopSearch)
        break;

      if (m != none)
        bestMove = m;
    }
    return bestMove;
  }
}
