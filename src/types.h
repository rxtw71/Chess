#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <iostream>



namespace Engine {
  using Bitboard = uint64_t;
  using Key = uint64_t;

  constexpr int MAX_MOVES = 256;
  constexpr int MAX_PLY = 246;

  enum Color {
    WHITE, BLACK,
    COLOR_NB = 2
  };

  enum CastlingRights {
    NO_CASTLING,
    WHITE_OO,
    WHITE_OOO = WHITE_OO << 1,
    BLACK_OO = WHITE_OO << 2,
    BLACK_OOO = WHITE_OO << 3,

    KING_SIDE = WHITE_OO | BLACK_OO,
    QUEEN_SIDE = WHITE_OOO | BLACK_OOO,
    WHITE_CASTLING = WHITE_OO | WHITE_OOO,
    BLACK_CASTLING = BLACK_OO | BLACK_OOO,
    ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,

    CASTLING_RIGHTS_NB = 16
  };

  enum Bound {
    BOUND_NONE,
    BOUND_UPPER,
    BOUND_LOWER,
    BOUND_EXACT = BOUND_LOWER | BOUND_UPPER
  };

  using Value = int;

  constexpr Value VALUE_ZERO = 0;
  constexpr Value VALUE_DRAW = 0;
  constexpr Value VALUE_NONE = 32002;
  constexpr Value VALUE_INFINITE = 32001;

  constexpr Value VALUE_MATE = 32000;
  constexpr Value VALUE_MATE_IN_MAX_PLY = VALUE_MATE - MAX_PLY;
  constexpr Value VALUE_MATED_IN_MAX_PLY = -VALUE_MATE_IN_MAX_PLY;

  constexpr Value VALUE_TB = VALUE_MATE_IN_MAX_PLY - 1;
  constexpr Value VALUE_TB_WIN_IN_MAX_PLY  = VALUE_TB - MAX_PLY;
  constexpr Value VALUE_TB_LOSS_IN_MAX_PLY = -VALUE_TB_WIN_IN_MAX_PLY;

  constexpr bool is_valid (Value value) {
    return value != VALUE_NONE;
  }
  constexpr bool is_win (Value value) {
    assert(is_valid(value));
    return value >= VALUE_TB_WIN_IN_MAX_PLY;
  }
  constexpr bool is_loss (Value value) {
    assert(is_valid(value));
    return value <= VALUE_TB_LOSS_IN_MAX_PLY;
  }

  // identify the material on the board.
  constexpr Value PawnValue   = 208;
  constexpr Value KnightValue = 781;
  constexpr Value BishopValue = 825;
  constexpr Value RookValue   = 1276;
  constexpr Value QueenValue  = 2538;

  //bits are aligned as color + piece  for easier op in fut.

  enum PieceType {
    NO_PIECE_TYPE,
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES = 0,
    PIECE_TYPE_NB = 8
  };

  enum Piece {
    NO_PIECE,
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB = 16
  };

  constexpr Value PieceValue [PIECE_NB] = {
    VALUE_ZERO, 
    PawnValue, KnightValue, BishopValue, RookValue, QueenValue, VALUE_ZERO,
    VALUE_ZERO, VALUE_ZERO,
    PawnValue, KnightValue, BishopValue, RookValue, QueenValue, VALUE_ZERO
  };

  enum Square : int {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,

    Sq0 = 0, SQUARE_NB = 64
  };

  enum Direction : int {
    NORTH = 8,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTH_EAST = NORTH + EAST,
    NORTH_WEST = NORTH + WEST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST
  };

  enum File : int {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
    FILE_NB
  };
  enum Rank : int {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
    RANK_NB
  };

    #define ENABLE_INC_OPS_ON(T) \
      inline T& operator++(T& d) {return d =  T(int(d) + 1);} \
      inline T& operator--(T& d) {return d = T(int(d) - 1);}

      ENABLE_INC_OPS_ON(PieceType);
      ENABLE_INC_OPS_ON(Square);
      ENABLE_INC_OPS_ON(File);
      ENABLE_INC_OPS_ON(Rank);

    #undef ENABLE_INC_OPS_ON

  constexpr Direction operator+ (Direction d1, Direction d2) {return Direction(int(d1) + int(d2));}
  constexpr Direction operator- (Direction d1, Direction d2) {return Direction(int(d1)- int(d2));}

  constexpr Square operator+ (Square s, Direction d) {return Square(int(s) + int(d));}
  constexpr Square operator- (Square s, Direction d) {return Square(int(s) - int(d));}
  inline Square& operator+= (Square& s, Direction d) {return s = s + d;}
  inline Square& operator-= (Square& s, Direction d) {return s = s - d;}

  // Toggle color
  constexpr Color operator~(Color c) { return Color(c ^ BLACK);}
  // Swap A1 <-> A8
  constexpr Square flip_rank(Square s) { return Square(s ^ a8);}

  // Swap A1 <-> H1
  constexpr Square flip_file(Square s) { return Square(s ^ h1);}

  // Swap color of piece B_KNIGHT <-> W_KNIGHT
  constexpr Piece operator~(Piece pc) { return Piece(pc ^ 8);}

  constexpr CastlingRights operator&(Color c, CastlingRights cr) {
      return CastlingRights((c == WHITE ? WHITE_CASTLING : BLACK_CASTLING) & cr);
  }

  constexpr Value mate_in(int ply) { return VALUE_MATE - ply;}

  constexpr Value mated_in(int ply) { return -VALUE_MATE + ply;}

  constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f);}

  constexpr Piece make_piece(Color c, PieceType pt) { return Piece((c << 3) + pt);}

  constexpr PieceType type_of(Piece pc) { return PieceType(pc & 7);}

  inline Color color_of(Piece pc) {
      assert(pc != NO_PIECE);
      return Color(pc >> 3);
  }

  constexpr bool is_ok(Square s) { return s >= a1 && s <= h8;}

  constexpr File file_of(Square s) { return File(s & 7);}

  constexpr Rank rank_of(Square s) { return Rank(s >> 3);}

  constexpr Square relative_square(Color c, Square s) { return Square(s ^ (c * 56));}

  constexpr Rank relative_rank(Color c, Rank r) { return Rank(r ^ (c * 7));}

  constexpr Rank relative_rank(Color c, Square s) { return relative_rank(c, rank_of(s));}

  constexpr Direction pawn_push(Color c) { return c == WHITE ? NORTH : SOUTH;}


  // Based on a congruential pseudo-random number generator
  constexpr Key make_key(uint64_t seed) {
      return seed * 6364136223846793005ULL + 1442695040888963407ULL;
  }

  inline std::string squareStr (Square s) {
    std::string square;
    int sq = s;

    square.push_back('a' + (sq % 8));
    square.push_back('1' + (sq / 8));

    return square;
  }


  enum MoveType {
      NORMAL,
      PROMOTION  = 1 << 14,
      EN_PASSANT = 2 << 14,
      CASTLING   = 3 << 14
  };

  struct Move {
    Move() = default;
    constexpr explicit Move(uint16_t d) : data(d) {}

    constexpr Move(Square from, Square to) :
      data((from << 6) + to) {}

    template <MoveType T>
      static constexpr Move make(Square from, Square to, PieceType pt = KNIGHT) {
        return Move(T + ((pt - KNIGHT) << 12) + (from << 6) + to);
      }

    constexpr Square from_sq () const {
      assert(is_ok());
      return Square((data >> 6) & 0x3F);
    }
    constexpr Square to_sq () const {
      assert(is_ok());
      return Square(data & 0x3F);
    }

    constexpr int from_to () const {return data & 0xFFF;}
    constexpr MoveType type_of () const {return MoveType(data & (3 << 14));}
    constexpr PieceType promotion_type () const {return PieceType(((data >> 12) & 3) + KNIGHT);}

    constexpr bool is_ok () const {
      return data != none().data && data != null().data;
    }
    static constexpr Move none () {return Move(0);}
    static constexpr Move null () {return Move(65);}

    constexpr bool operator== (Move& m) const {return data == m.data;}
    constexpr bool operator!= (Move& m) const {return data != m.data;}

    constexpr explicit operator bool() const { return data != 0; }

    constexpr std::uint16_t raw () const {return data;}

    void print() const {
        static const char* pieceNames[] = {"", "Knight", "Bishop", "Rook", "Queen"};
        static const char* moveTypeNames[] = {"Normal", "Promotion", "EnPassant", "Castling"};

        int f = from_sq();
        int t = to_sq();

        std::cout << "Move: " 
                  << char('a' + (f % 8)) << (f / 8 + 1)   // from square
                  << " -> " 
                  << char('a' + (t % 8)) << (t / 8 + 1);  // to square

        int mt = (data >> 14) & 3;

        std::cout << " | Type: " << moveTypeNames[mt];

        if (mt == PROMOTION) {
            PieceType pt = promotion_type();
            std::cout << " | Promotes to: " << pieceNames[pt];
        }

        std::cout << "\n";
    }

    protected:
    std::uint16_t data;
  };


}
