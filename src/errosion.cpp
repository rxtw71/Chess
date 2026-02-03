#include "errosion.h"


namespace Leaf {

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
}
