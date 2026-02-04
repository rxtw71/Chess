#pragma once

#include "types.h"
#include "board.h"
#include "bitboard.h"

#include <vector>

namespace Leaf {

  struct MoveList {
    Move data[256];
    int count = 0;

    inline void add (const Move& m) {
      assert(count < 256);
      data[count++] = m;
    }
  };


  Bitboard whitePawnMoves (Board& b, Square s);
  Bitboard blackPawnMoves (Board& b, Square s);
  Bitboard whiteep (Board& b, Square s);
  Bitboard blackep (Board& b, Square s);
  Bitboard whitecastlingmoves (Board& b);
  Bitboard blackcastlingmoves (Board& b);

  template <PieceType pt>
    Bitboard pieceAttacks (Board& b, Square s) {
      if constexpr (pt == PAWN) {
        if (color_of(b.data[s]) == WHITE)
          return whitePawnMoves(b, s) | (PawnAttacks[WHITE][s] & b.Occupancy[BLACK]) | whiteep(b, s);
        else
          return blackPawnMoves(b, s) | (PawnAttacks[BLACK][s] & b.Occupancy[WHITE]) | blackep(b, s);
      }
      else if constexpr (pt == KING) {
        if (color_of(b.data[s]) == WHITE)
          return PsudoAttacks[KING][s] | whitecastlingmoves(b);
        else
          return PsudoAttacks[KING][s] | blackcastlingmoves(b);
      }
      else if constexpr (pt == KNIGHT)
        return PsudoAttacks[KNIGHT][s];
      else if constexpr (pt == BISHOP)
        return attacks_bb<BISHOP>(s, b.Occupancy[2]);
      else if constexpr (pt == ROOK)
        return attacks_bb<ROOK>(s, b.Occupancy[2]);
      else if constexpr (pt == QUEEN)
        return attacks_bb<QUEEN>(s, b.Occupancy[2]);
      else 
        return 0;
    }

  inline Bitboard pieceAttacks(Board& b, PieceType pt, Square s) {
    Bitboard attacks;

    switch (pt) {
      case PAWN : attacks = pieceAttacks<PAWN>(b, s); break;
      case KNIGHT : attacks = pieceAttacks<KNIGHT>(b, s); break;
      case BISHOP : attacks = pieceAttacks<BISHOP>(b, s); break;
      case ROOK : attacks = pieceAttacks<ROOK>(b, s); break;
      case QUEEN : attacks = pieceAttacks<QUEEN>(b, s); break;
      case KING : attacks = pieceAttacks<KING>(b, s); break;
      default : return 0;
    }

    Bitboard freindly = (b.turn == WHITE) ? b.Occupancy[WHITE] : b.Occupancy[BLACK];
    return attacks & ~freindly;
  }

  void PsudoMoves (Board& b, Color c, MoveList& list);
  void LegalMoves (Board& b, MoveList& list);
  int moveScore (Board& b, Move& m);

}
