#include "engine.h"
#include "board.h"
#include "evaluation.h"
#include "movegen.h"
#include "types.h"
#include <chrono>
#include <ctime>
#include <vector>

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

  //Gloabl tables
  PV pv;
  Killer killer;
  TT transposition;
  int SEARCHED_NODES;

  int NegaMax (Board& b, int depth, int ply, int alpha, int beta) {
    if (stopSearch.load(std::memory_order_relaxed) || timeUp())
      return 0;

    SEARCHED_NODES++;

    int ttscore;
    int alphaOrig = alpha;
    Bound iflag;
    if (transposition.probe(b.key, depth, ttscore, alpha, beta, iflag)) {
      return ttscore;
    }

    //Checkmate and stalemate conditions
    if (DrawGame(b))
      return VALUE_DRAW;

    MoveList list;
    LegalMoves(b, list);
    Move mtt = transposition.getMove(b.key);
    Move mpv = (pv.length[ply] > 0) ? pv.Table[ply][0] : Move::none();
    Move mk1 = killer.Table[0][ply];
    Move mk2 = killer.Table[1][ply];
    sortMoveList(b, list, mtt, mpv, mk1, mk2);

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

      int newDepth = depth - 1;

      if (i >= 3 && !b.givesCheck(list.data[i]) && (!b.isCapture(list.data[i]) && list.data[i] != mpv && list.data[i] != mk1 && list.data[i] != mk2))
        newDepth -= 1;
      if (newDepth < 0)
        newDepth = 0;
      Board::State state;
      b.MakeMove(list.data[i], state);
      score = -NegaMax(b, newDepth, ply + 1, -beta, -alpha);
      b.UnmakeMove(list.data[i]);
     
      if (stopSearch.load(std::memory_order_relaxed) || timeUp())
        return 0;

      if (score > bestScore) {
        bestScore = score;
        bestMove = list.data[i];

        pv.Table[ply][0] = bestMove;
        for (int j = 0; j < pv.length[ply + 1]; j++) {
          pv.Table[ply][j + 1] = pv.Table[ply + 1][j];
        }
        pv.length[ply] = pv.length[ply + 1] + 1;
      }
      if (score >= alpha) {
        alpha = score;
      }
      if (alpha >= beta) {
        if (!b.isCapture(list.data[i])) {
          killer.Table[1][ply] = killer.Table[0][ply];
          killer.Table[0][ply] = list.data[i];
        }
        break;
      }
    }
    Bound flag;
    if (bestScore <= alphaOrig) flag = BOUND_UPPER;
    else if (bestScore >= beta) flag = BOUND_LOWER;
    else flag = BOUND_EXACT;

    transposition.store(b.key, depth, bestScore, flag, bestMove);

    return bestScore;
  }

  Move FindBestMove (Board& b, int depth, int alpha, int beta) {
    
    Move bestMove = Move::none();
    if (stopSearch.load(std::memory_order_relaxed) || timeUp())
      return bestMove;

    int bestScore = -VALUE_INFINITE;

    int ttscore; Bound flag;
    Move ttmove = transposition.getMove(b.key);
    if (transposition.probe(b.key, depth, ttscore, alpha, beta, flag)) {
      if (flag == BOUND_EXACT) return ttmove;
      else if (flag == BOUND_LOWER && ttscore > alpha) alpha = ttscore;
      else if (flag == BOUND_UPPER && ttscore < beta) beta = ttscore;
      if (alpha >= beta) return ttmove;
    }

    MoveList list;
    LegalMoves(b, list);
    Move mpv = (pv.length[0] > 0) ? pv.Table[0][0] : Move::none();
    Move mk1 = killer.Table[0][0];
    Move mk2 = killer.Table[1][0];
    sortMoveList(b, list, ttmove, mpv, mk1, mk2);
    Move none = Move::none();


    int score;
    for (int i = 0; i < list.count; i++) {
      Board::State state;
      b.MakeMove(list.data[i], state);
      score = -NegaMax(b, depth - 1, 1, -beta, -alpha);
      b.UnmakeMove(list.data[i]);

      if (stopSearch.load(std::memory_order_relaxed) || timeUp())
        return bestMove;

      if (score > bestScore) {
        bestScore = score;
        bestMove = list.data[i];

        pv.Table[0][0] = bestMove;
        for (int j = 0; j < pv.length[0 + 1]; j++) {
          pv.Table[0][j + 1] = pv.Table[0 + 1][j];
        }
        pv.length[0] = pv.length[0 + 1] + 1;
      }
      if (score >= alpha) {
        alpha = score;
      }
      if (alpha >= beta) {
        if (!b.isCapture(list.data[i])) {
          killer.Table[1][0] = killer.Table[0][0];
          killer.Table[0][0] = list.data[i];
        }
        break;
      }
    }
    transposition.store(b.key, depth, bestScore, BOUND_EXACT, bestMove);

    return bestMove;
  }


  Move SearchMove (Board& b, int maxDepth) {
    stopSearch.store(false);
    startSearch = Clock::now();

    Move none = Move::none();
    Move bestMove = Move::none();

    //clear tables
    pv.clear();
    killer.clear();

    SEARCHED_NODES = 0;
    for (int depth = 1; depth <= maxDepth; depth++) {

      Move m = FindBestMove(b, depth, -VALUE_INFINITE, VALUE_INFINITE);

      if (stopSearch.load(std::memory_order_relaxed) || timeUp())
        return bestMove;
      bestMove = m;
    }

    std::cout << "Searched nodes->" << SEARCHED_NODES << std::endl;
    return bestMove;
  }

  std::vector<Move> readPV() {
    std::vector<Move> pvs;
    for (int i = 0; i < pv.length[0]; i++) {
      pvs.push_back(pv.Table[0][i]);
    }
    return pvs;
  }
  inline bool capturegood (Board& b, Move& m) {
    Square x = m.from_sq();
    Square y = m.to_sq();
    Piece attacker = b.data[x];
    Piece victim = b.data[y];
    return PieceValue[victim] + PawnValue >=  PieceValue[attacker];
  }



  int moveValue (Board& b, Move& m,Move& tt,  Move& pv, Move& k1, Move& k2) {
    int score = 0;
    if (m == tt) return 100000;
    if (m == pv) return 9999;
    if (m == k1) return 8000;
    if (m == k2) return 700;
    if (m.type_of() == EN_PASSANT) score += 500;
    else if (b.isCapture(m) && capturegood(b, m)) score += 650;
    else if (b.isCapture(m) && !capturegood(b, m)) score += 200;
    if (b.givesCheck(m)) score += 600;

    return score;
  }
  void sortMoveList (Board& b, MoveList& list, Move& tt, Move& pv, Move& k1, Move& k2) {
    for (int i = 0; i < list.count - 1; i++) {
      for (int j = i + 1; j < list.count; j++) {
        if (moveValue(b, list.data[i], tt, pv, k1, k2) < moveValue(b, list.data[j], tt, pv, k1, k2)) {
          Move temp = list.data[i];
          list.data[i] = list.data[j];
          list.data[j] = temp;
          //std::swap(list.data[i], list.data[j]);
        }
      }
    }
  }


}
