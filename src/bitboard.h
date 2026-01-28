#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <cmath>
#include <random>

#include "types.h"

namespace Engine {
  namespace Bitboards {
    void init();
    std::string pretty ();
  }

  constexpr Bitboard FileABB = 0x0101010101010101ULL;
  constexpr Bitboard FileBBB = FileABB << 1;
  constexpr Bitboard FileCBB = FileABB << 2;
  constexpr Bitboard FileDBB = FileABB << 3;
  constexpr Bitboard FileEBB = FileABB << 4;
  constexpr Bitboard FileFBB = FileABB << 5;
  constexpr Bitboard FileGBB = FileABB << 6;
  constexpr Bitboard FileHBB = FileABB << 7;
 
  constexpr Bitboard Rank1BB = 0xFF;
  constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
  constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
  constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
  constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
  constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
  constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
  constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

  extern uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

  extern Bitboard PsudoAttacks[PIECE_TYPE_NB][SQUARE_NB];
  extern Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];

  struct Magic {
    Bitboard mask;
    Bitboard* attacks;
    Bitboard magic;
    unsigned int shift;
    
    unsigned int index (Bitboard occupied) {
      return static_cast<unsigned int> (((occupied & mask) * magic) >> shift);
    }

    Bitboard attacks_bb (Bitboard occupied) {
      return attacks[index(occupied)];
    }
  };

  extern Magic Magics [SQUARE_NB][2];
  constexpr Bitboard square_bb (Square s) {
    return (1ULL << s);
  }

  inline Bitboard operator& (Bitboard b, Square s) {return b & square_bb(s);}
  inline Bitboard operator| (Bitboard b, Square s) {return b | square_bb(s);}
  inline Bitboard operator^ (Bitboard b, Square s) {return b ^ square_bb(s);}
  inline Bitboard& operator&= (Bitboard& b, Square s) {return b &= square_bb(s);}
  inline Bitboard& operator|= (Bitboard& b, Square s) {return b |= square_bb(s);}
  inline Bitboard& operator^= (Bitboard& b, Square s) {return b ^= square_bb(s);}

  inline Bitboard operator& (Square s, Bitboard b) {return b & s;}
  inline Bitboard operator| (Square s, Bitboard b) {return b | s;}
  inline Bitboard operator^ (Square s, Bitboard b) {return b ^ s;}

  /*---------------Essenitial functions-------------*/
  //Get rank and file of Square
  constexpr Bitboard rank_bb (Rank r) {return Rank1BB << (8 * r);}
  constexpr Bitboard file_bb (File f) {return FileABB << f;}
  constexpr Bitboard rank_bb (Square s) {return rank_bb(rank_of(s));}
  constexpr Bitboard file_bb (Square s) {return file_bb (file_of(s));}

  //distance functions...
  template<typename T1 = Square>
    inline int distance (Square x, Square y);

  template<>
    inline int distance<Rank> (Square x, Square y) {
      return std::abs(rank_of(x) - rank_of(y));
    }
  template<>
    inline int distance<File> (Square x, Square y) {
      return std::abs(file_of(x) - file_of(y));
    }

  template<>
    inline int distance<Square> (Square x, Square y) {
      return SquareDistance[x][y];
    }

  //shift
  template<Direction d>
    constexpr Bitboard shift (Bitboard b) {
      return d == NORTH ? b << 8 :
             d == SOUTH ? b >> 8 :
             d == NORTH + NORTH ? b << 16 :
             d == SOUTH + SOUTH ? b >> 16 :
             d == EAST ? (b & ~FileHBB) << 1 :
             d == WEST ? (b & ~FileABB) >> 1 :
             d == NORTH_WEST ? (b & ~FileABB) << 7 :
             d == NORTH_EAST ? (b & ~FileHBB) << 9 :
             d == SOUTH_EAST ? (b & ~FileHBB) >> 7 :
             d == SOUTH_WEST ? (b & ~FileABB) >> 9 : 0 ;
    }
/*--------------PawnAttacks-----------*/
  //give squares attacked by pawn from given bitborard of pawns 
  //and for per square of pawn
  template<Color c>
    constexpr Bitboard pawn_attacks_bb (Bitboard b) {
      return c == WHITE ? shift<NORTH_EAST>(b) | shift<NORTH_WEST>(b) :
                          shift<SOUTH_EAST>(b) | shift<SOUTH_WEST>(b) ;
    }

  inline Bitboard pawn_attacks_bb (Color c, Square s) {
    return PawnAttacks[c][s];
  }
/*-----------------------------*/
  template <PieceType pt>
    inline Bitboard attacks_bb (Square s) {
      assert((pt != PAWN) && is_ok(s));
      return PsudoAttacks[pt][s];
    }

  template <PieceType pt>
    inline Bitboard attacks_bb (Square s, Bitboard occupied) {
      assert((pt != PAWN) && is_ok(s));
      switch (pt) {
        case BISHOP :
        case ROOK :
          return Magics[s][pt - BISHOP].attacks_bb(occupied);
        case QUEEN :
          return attacks_bb<BISHOP> (s, occupied) | attacks_bb<ROOK> (s, occupied);
        default : 
          return PsudoAttacks[pt][s];
      }
    }

  inline Bitboard attacks_bb (PieceType pt, Square s, Bitboard occupied) {
    assert ((pt != PAWN) && is_ok(s));
    switch (pt) {
      case BISHOP :
        return attacks_bb<BISHOP>(s, occupied);
      case ROOK :
        return attacks_bb<ROOK>(s, occupied);
      case QUEEN :
        return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
      default :
        return PsudoAttacks[pt][s];
    }
  }

  inline Square lsb (Bitboard b) {
    return Square(__builtin_ctzll(b));
  }
  inline Square msb (Bitboard b) {
    return Square(__builtin_clzll(b));
  }
  inline int popcount (Bitboard b) {
    return __builtin_popcountll(b);
  }
  inline Square pop_lsb (Bitboard& b) {
    const Square s = lsb(b);
    b &= b - 1;
    return s;
  }
}
