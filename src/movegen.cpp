#include "movegen.h"
#include "types.h"
#include <iostream>
#include <utility>

namespace Leaf {
  Bitboard whitePawnMoves (Board& b, Square s) {
    Bitboard moves = 0ULL;
    Square one = s + NORTH;

    if (!b.squareEmpty(one))
      return moves;

    moves |= one;

    if (rank_of(s) == RANK_2) {
      one += NORTH;
      if (b.squareEmpty(one))
        moves |= one;
    }

    return moves;
 }
  Bitboard blackPawnMoves (Board& b, Square s) {
    Bitboard moves = 0ULL;
    Square one = s + SOUTH;

    if (!b.squareEmpty(one))
      return moves;

    moves |= one;

    if (rank_of(s) == RANK_7) {
      one += SOUTH;
      if (b.squareEmpty(one))
        moves |= one;
    }

    return moves;
  }

  Bitboard whiteep (Board& b, Square s) {
    if (b.Enpassant == Sq0)
      return 0;

    Bitboard attacks = PawnAttacks[WHITE][s];
    if (attacks & b.Enpassant)
      return square_bb(b.Enpassant);

    return 0;
  }
  Bitboard blackep (Board& b, Square s) {
    if (b.Enpassant == Sq0)
      return 0;

    Bitboard attacks = PawnAttacks[BLACK][s];
    if (attacks & b.Enpassant)
      return square_bb(b.Enpassant);

    return 0;
  }

  static constexpr Bitboard kingspathwhite = square_bb(e1) | square_bb(f1) | square_bb(g1);
  static constexpr Bitboard queenspathwhite = square_bb(e1) | square_bb(d1) | square_bb(c1);
  static constexpr Bitboard kingspathblack = square_bb(e8) | square_bb(f8) | square_bb(g8);
  static constexpr Bitboard queenspathblack = square_bb(e8) | square_bb(d8) | square_bb(c8);
  
  Bitboard whitecastlingmoves (Board& b) {

    if (!(WHITE_CASTLING & b.CastleRights)) {
      return 0;
    }

    Bitboard moves = 0;

    if (b.CastleRights & WHITE_OO)
      if (b.squareEmpty(f1) && b.squareEmpty(g1) && !b.squareAttacked(BLACK, kingspathwhite ))
        moves |= g1;

    if (b.CastleRights & WHITE_OOO)
      if (b.squareEmpty(d1) && b.squareEmpty(c1) && b.squareEmpty(b1) && !b.squareAttacked(BLACK, queenspathwhite ))
        moves |= c1;

    return moves;
  }
  Bitboard blackcastlingmoves (Board& b) {
    if (!(BLACK_CASTLING & b.CastleRights))
      return 0;

    Bitboard moves = 0;

    if (b.CastleRights & BLACK_OO)
      if (b.squareEmpty(f8) && b.squareEmpty(g8) && !b.squareAttacked(WHITE, kingspathblack ))
        moves |= g8;

    if (b.CastleRights & BLACK_OOO)
      if (b.squareEmpty(d8) && b.squareEmpty(c8) && b.squareEmpty(b8) && !b.squareAttacked(WHITE, queenspathblack ))
        moves |= c8;

    return moves;
  }


  void PsudoMoves (Board& b, Color c, MoveList& list) {
    
    int psudocount = 0;
    Bitboard pieces;
    Rank promotionRank;
    Bitboard castlesqs;

    if (c == WHITE) {
      pieces = b.Occupancy[WHITE];
      promotionRank = RANK_8;
      castlesqs = square_bb(g1) | square_bb(c1);
    } else {
      pieces = b.Occupancy[BLACK];
      promotionRank = RANK_1;
      castlesqs = square_bb(g8) | square_bb(c8);
    }

    while (pieces) {
      Square from = pop_lsb(pieces);
      Piece p = b.data[from];
      PieceType pt = type_of(p);
      Bitboard attacks = pieceAttacks(b, pt, from);

      while (attacks) {
        Square to = pop_lsb(attacks);

        if (pt == PAWN) {
          //promo flags
          if (rank_of(to) == promotionRank) {
            list.add(Move::make<PROMOTION>(from, to, KNIGHT));
            list.add(Move::make<PROMOTION>(from, to, BISHOP));
            list.add(Move::make<PROMOTION>(from, to, ROOK));
            list.add(Move::make<PROMOTION>(from, to, QUEEN));
          }
          else if (to == b.Enpassant) {
            list.add(Move::make<EN_PASSANT>(from, to));
          }
          else 
            list.add(Move::make<NORMAL>(from, to));
        }
        else if (pt == KING) {
          if (to  & castlesqs && (from == e1 || from == e8)) {
            list.add(Move::make<CASTLING>(from, to));
          }
          else {
            list.add(Move::make<NORMAL>(from, to));
          }
        }
        else
          list.add(Move::make<NORMAL>(from, to));


      }

    }
  }
  void LegalMoves(Board& b, MoveList &list) {
    MoveList psudo;
    PsudoMoves(b, b.turn, psudo);
     for (int i = 0; i < psudo.count; i++) {
       Board::State st;
       b.MakeMove(psudo.data[i], st);

       if (!b.kingInCheck())
         list.add(psudo.data[i]);
       b.UnmakeMove(psudo.data[i]);
     }
  }
  
}
