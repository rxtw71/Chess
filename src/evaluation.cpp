#include "evaluation.h"
#include "bitboard.h"
#include "board.h"
#include "types.h"

namespace Leaf {
  //basic material vbased evaluation
  int materialAdvantage (Board& b) {
    int score = 0;

    score += popcount(b.piecebb[W_PAWN]) * PawnValue;
    score -= popcount(b.piecebb[B_PAWN]) * PawnValue;
    score += popcount(b.piecebb[W_KNIGHT]) * KnightValue;
    score -= popcount(b.piecebb[B_KNIGHT]) * KnightValue;
    score += popcount(b.piecebb[W_BISHOP]) * BishopValue;
    score -= popcount(b.piecebb[B_BISHOP]) * BishopValue;
    score += popcount(b.piecebb[W_ROOK]) * RookValue;
    score -= popcount(b.piecebb[B_ROOK]) * RookValue;
    score += popcount(b.piecebb[W_QUEEN]) * QueenValue;
    score -= popcount(b.piecebb[B_QUEEN]) * QueenValue;

    return score;
  }
  
  int mobilityAdvantage (Board& b) {
    int score = 0;
    int mobscore[] = {0, 0, 4, 3, 0, 0, 0};

    Piece p[] = {W_KNIGHT, B_KNIGHT, W_BISHOP, B_BISHOP};
    for (auto pp : p) {
      Bitboard piece = b.piecebb[pp];
      while (piece) {
        Square s = pop_lsb(piece);
        Bitboard attacks = attacks_bb(type_of(pp), s, b.Occupancy[2]);
        int x = (color_of(pp) == WHITE) ? 1 : -1;
        score += popcount(attacks) * x * mobscore[type_of(pp)];

      }
    }

    return score;
  }

  constexpr int phaseValue[PIECE_TYPE_NB] = {
    0, 0, 1, 1, 2, 4, 0
  };

  const int PawnPST[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     5,  10,  10, -20, -20,  10,  10,   5,
     5,  -5, -10,   0,   0, -10,  -5,   5,
     0,   0,   0,  20,  20,   0,   0,   0,
     5,   5,  10,  25,  25,  10,   5,   5,
    10,  10,  20,  30,  30,  20,  10,  10,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,   0,   0,   0,   0,   0,   0
  };
  constexpr int KnightPST[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
  };
  const int BishopPST[64] = {
  -20, -10, -10, -10, -10, -10, -10, -20,
  -10,   0,   0,   0,   0,   0,   0, -10,
  -10,   0,   5,  10,  10,   5,   0, -10,
  -10,   5,   5,  10,  10,   5,   5, -10,
  -10,   0,  10,  10,  10,  10,   0, -10,
  -10,  10,  10,  10,  10,  10,  10, -10,
  -10,   5,   0,   0,   0,   0,   5, -10,
  -20, -10, -10, -10, -10, -10, -10, -20
  };
  const int KingPSTOpening[64] = {
    10, 20, 20, 0, 0, 5, 20, 10,
    5, 10, 5, 0, 0, 5, 10, 5, 
    -10, -10, -10, -10, -10, -10, -10,
    -10, -10, -10, -10, -10, -10, -10,
    -10, -10, -10, -10, -10, -10, -10,
    -20, -20, -20, -20, -20, -20, -20,
    -20, -20, -20, -20, -20, -20, -20,
    -30, -30, -30, -30, -30, -30, -30,
  };
      
  //Pawn evaluation r`fucntions
  constexpr Value DOUBLEPAWNS_PEN = 5;
  constexpr Value ISOLATEDPAWN_PEN = 10;
  constexpr Value PASSEDPAWN = 8;
  int PawnStructureEval (Bitboard wp, Bitboard bp) {
    int score = 0;

    //counts pawn per file
    int wpfile_cc[8]; int bpfile_cc[8];
    for (int i = 0; i < 8; i++) {
      wpfile_cc[i] = popcount(wp & FileMaskBB[i]);
      bpfile_cc[i] = popcount(bp & FileMaskBB[i]);
    }

    //Penlty for stacked pawns and isolatedolated pawns
    for (int i = 0; i < 8; i++) {
      if (wpfile_cc[i] > 1)
        score -= (wpfile_cc[i] - 1) * DOUBLEPAWNS_PEN;
      if (bpfile_cc[i] > 1)
        score += (bpfile_cc[i] - 1) * DOUBLEPAWNS_PEN;

      if (wpfile_cc[i] != 0) {
        bool isolated = true;
        if (i > 0 && wpfile_cc[i - 1] > 0)
          isolated = false;
        if (i < 7 && wpfile_cc[i + 1] > 0)
          isolated = false;
        if (isolated)
          score -= ISOLATEDPAWN_PEN;
      }
      if (bpfile_cc[i] != 0) {
        bool isolated = true;
        if (i > 0 && bpfile_cc[i - 1] > 0)
          isolated = false;
        if (i < 7 && bpfile_cc[i + 1] > 0)
          isolated = false;
        if (isolated)
          score += ISOLATEDPAWN_PEN;
      }

    }

    return score;
  }
  




  int GamePhase (Board& b) {
    int phase = 0;

    for (Square s = a1; s <= h8; ++s) {
      Piece p = b.getPiece(s);
      if (p == NO_PIECE)
        continue;

      PieceType pt = type_of(p);
      phase += phaseValue[pt];
    }

    return phase;
  }

  int KnightEval (Board& b) {
    int score = 0;

    Bitboard knight = b.piecebb[W_KNIGHT];

    while (knight) {
      Square s = pop_lsb(knight);
      score += KnightPST[s];
    }
    knight = b.piecebb[B_KNIGHT];

    while (knight) {
      Square s = pop_lsb(knight);
      s = flip_rank(s);
      score -= KnightPST[s];
    }

    return score;
  }
  int PawnEval (Board& b) {
    int score = 0;

    Bitboard pawn = b.piecebb[W_PAWN];
    while (pawn) {
      Square s = pop_lsb(pawn);
      score += PawnPST[s];
    }
    pawn = b.piecebb[B_PAWN];
    while (pawn) {
      Square s = pop_lsb(pawn);
      s = flip_rank(s);
      score -= PawnPST[s];
    }

    return score;
  }
  int BishopEval (Board& b) {
    int score = 0;

    Bitboard bishop = b.piecebb[W_BISHOP];
    while (bishop) {
      Square s = pop_lsb(bishop);
      score += BishopPST[s];
    }
    bishop = b.piecebb[B_BISHOP];
    while (bishop) {
      Square s = pop_lsb(bishop);
      s = flip_rank(s);
      score -= BishopPST[s];
    }

    return score;
  }

  int KingEval (Board& b) {
    int score = 0;
    Bitboard wKing = b.piecebb[W_KING];
    Bitboard bKING = b.piecebb[B_KING];

    Square wk = pop_lsb(wKing);
    Square bk = pop_lsb(bKING);
    bk = flip_rank(bk);

    score += KingPSTOpening[wk];
    score -= KingPSTOpening[bk];

    return score;
  }

  int Eval (Board& b) {
    int score = 0;

    score += materialAdvantage(b);
    score += KnightEval(b);
    score += PawnEval(b);
    score += PawnStructureEval(b.piecebb[W_PAWN], b.piecebb[B_PAWN]);
    score += BishopEval(b);
    score += KingEval(b);


    score = (b.turn == WHITE) ? score : -score;
    return score;// + KingAct(b);
  } 
}
