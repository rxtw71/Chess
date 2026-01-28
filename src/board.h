#pragma once
#include "types.h"
#include "bitboard.h"
#include "hash.h"

#include <string>

namespace Engine {
  class Board {
    public:
    Bitboard piecebb[PIECE_NB];
    Bitboard Occupancy[3];
    Piece data[SQUARE_NB];

    Hash key;
    Color turn;
    uint8_t CastleRights;
    Square Enpassant;
    int halfMove;

    struct State {
      Hash key;
      uint8_t CastleRights;
      Square Enpassant;
      int halfMove;
      Piece Captured;
    };
    std::vector<State> history;
    

    public:

    void clearBoard ();
    void init ();
    bool loadFEN (const std::string& fen);
    void print();
    void updateOccupancy ();
    bool squareAttacked (Color by, Bitboard sqs);

    Hash compute_hash ();


    //state functions
    inline Piece getPiece (Square s) {
      return data[s];
    }
    inline Square getKingSq (Color c) {
      Bitboard king = c == WHITE ? piecebb[W_KING] : piecebb[B_KING];
      return lsb(king);
    }
    inline Color colorofPiece (Piece p) {
      return (p <= W_KING) ? WHITE : BLACK;
    }

    inline void setPiece (Piece p, Square s) {
      data[s] = p;
      Bitboard sb = square_bb(s);
      piecebb[p] |= sb;
      Occupancy[colorofPiece(p)] |= sb;
      Occupancy[2] |= sb;
      if (p != NO_PIECE)
        key ^= piecehash(p, s);
    }
    inline void removePiece (Piece p, Square s) {
      data[s] = NO_PIECE;
      Bitboard sb = ~square_bb(s);
      piecebb[p] &= sb;
      Occupancy[colorofPiece(p)] &= sb;
      Occupancy[2] &= sb;
      if (p != NO_PIECE)
        key ^= piecehash(p, s);
    }
    inline void removePiece (Square s) {
      Piece p = getPiece(s);
      removePiece(p, s);
    }
    inline bool squareEmpty (Square s) {
      return data[s] == NO_PIECE;
    }
    inline bool kingInCheck () {
      Color by; Bitboard king;

      if (turn == BLACK) {
        by = BLACK;
        king = piecebb[W_KING];
      } else {
        by = WHITE;
        king = piecebb[B_KING];
      }
      return squareAttacked(by, king);
    }
    inline bool kingInCheck (Color c) {
      Color by; Bitboard king;
      if (c == WHITE) {
        king = piecebb[W_KING];
        by = BLACK;
      } else {
        king = piecebb[B_KING];
        by = WHITE;
      }
      return squareAttacked(by, king);
    }

    //
    inline bool isCapture (Move& m) {
      if (data[m.to_sq()] != NO_PIECE) return true;
      else return false;
    }
    inline PieceType capturedpt (Move& m) {
      if (m.type_of() == EN_PASSANT) return PAWN;
      else return type_of(data[m.to_sq()]);
    }
    inline bool givesCheck (Move& m) {
      Board::State st;
      MakeMove(m, st);
      bool check = kingInCheck(turn);
      UnmakeMove(m);
      return check;
    }


    void moveRooks (Square to, int recover = -1);
    void capturedEP (Square to, int recover = -1);
    void removeRookCastle (Square from, Square to);




    void MakeMove (const Move& move, State& state);
    void UnmakeMove (const Move& move);
    void MakeNullMove (State& state);
    void UnmakeNULLMove (State& state);


  };
}

