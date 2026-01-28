#include "board.h"
#include "hash.h"
#include "types.h"
#include <cstdint>
#include <string>
#include <sstream>
#include <cctype>
#include <iostream>

namespace Engine {


  void Board::clearBoard () {
    for (int i = 0; i < PIECE_NB; i++) {
      piecebb[i] = 0;
    }
    key = 0;
    CastleRights = 0;
    turn = WHITE;
    Enpassant = Sq0;
    halfMove = 0;
    history.clear();

    for (Piece& p : data) p = NO_PIECE;

    updateOccupancy ();
    
  }
  void Board::updateOccupancy () {
    Occupancy[0] = 0;
    Occupancy[1] = 0;
    for (int i = 1; i <= 6; i++) {
      Occupancy[0] |= piecebb[i];
      Occupancy[1] |= piecebb[i + 8];
    }
    Occupancy[2] = Occupancy[0] | Occupancy[1];

    for (Square s = a1; s < SQUARE_NB; ++s) {
      data[s] = NO_PIECE;
      for (int p = 0; p < PIECE_NB; p++)
        if (piecebb[p] & s) {
          data[s] = Piece(p);
          break;
        }
    }
  }
  


  void Board::init () {
    std::string pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (loadFEN(pos)) {
      std::cout << "Startpos loaded" << std::endl;
    }
  }

  bool Board::loadFEN (const std::string& fen) {

    std::istringstream iss (fen);
    std::string placement, side, castling, ep, halfMove;

    if (!(iss >> placement >> side >> castling >> ep >> halfMove)) return false;

    clearBoard();

    Square sq = a8;

    for (char c : placement) {
      if (c == '/') {
        sq -= Direction(16);
      } else if (std::isdigit(c)) {
        sq += Direction(c - '0');
      } else {
        Piece p = NO_PIECE;

        switch (c) {
          case 'P' : p = W_PAWN; break;
          case 'R' : p = W_ROOK; break;
          case 'N' : p = W_KNIGHT; break;
          case 'B' : p = W_BISHOP; break;
          case 'Q' : p = W_QUEEN; break;
          case 'K' : p = W_KING; break;
          case 'p' : p = B_PAWN; break;
          case 'r' : p = B_ROOK; break;
          case 'n' : p = B_KNIGHT; break;
          case 'b' : p = B_BISHOP; break;
          case 'q' : p = B_QUEEN; break;
          case 'k' : p = B_KING; break;
          default : return false;
        }

        setPiece (p, sq);
        sq += Direction(1);

      }
    }

    turn = (side == "w") ? WHITE : BLACK;

    if (castling.find('K') != std::string::npos) CastleRights |= WHITE_OO;
    if (castling.find('Q') != std::string::npos)  CastleRights |= WHITE_OOO;
    if (castling.find('k') != std::string::npos) CastleRights |= BLACK_OO;
    if (castling.find('q') != std::string::npos) CastleRights |= BLACK_OOO;

    if (ep == "-") {
      Enpassant = Sq0;
    } else {
      int file = ep[0] - 'a';
      int rank = ep[1] - '1';
      Enpassant = Square(int(rank * 8 + file));
    }

    halfMove = std::stoi(halfMove);

    updateOccupancy ();
    key = compute_hash();
    return true;
  }

  void Board::print() {
    for (int r = 7; r >= 0; r--)
      for (int f = 0; f < 8; f++) {
        Square s = Square(r * 8 + f);
        Piece p = data[s];
        char c;

        switch (p) {
          case W_PAWN : c = 'P'; break;
          case W_KNIGHT : c = 'N'; break;
          case W_BISHOP : c = 'B'; break;
          case W_ROOK : c = 'R'; break;
          case W_QUEEN : c = 'Q'; break;
          case W_KING : c = 'K'; break;
          case B_PAWN : c = 'p'; break;
          case B_KNIGHT : c = 'n'; break;
          case B_BISHOP : c = 'b'; break;
          case B_ROOK : c = 'r'; break;
          case B_QUEEN : c = 'q'; break;
          case B_KING : c = 'k'; break;
          default : c = '.';
        }

        if (file_of(s) == FILE_A) std::cout << r + 1 << ' ';
        std::cout << c << " " ;
        if (file_of(s) == FILE_H) std::cout << '\n';



      }
    std::cout << "X A B C D E F G H\n" << std::endl;
  }

  bool Board::squareAttacked (Color by, Bitboard sqs) {
    Bitboard king, queen, rook, bishop, knight, pawn;
    if (by == WHITE) {
      king = piecebb[W_KING];
      queen = piecebb[W_QUEEN];
      rook = piecebb[W_ROOK];
      bishop = piecebb[W_BISHOP];
      knight = piecebb[W_KNIGHT];
      pawn = piecebb[W_PAWN];
    } else {
      king = piecebb[B_KING];
      queen = piecebb[B_QUEEN];
      rook = piecebb[B_ROOK];
      bishop = piecebb[B_BISHOP];
      knight = piecebb[B_KNIGHT];
      pawn = piecebb[B_PAWN];
    }
    //fuuuuuuuuuuu
    Bitboard attacks = 0ULL;

    while (knight) {
      Square sq = pop_lsb(knight);
      attacks |= PsudoAttacks[KNIGHT][sq];
    }

    while (rook) {
      Square sq = pop_lsb(rook);
      attacks |= attacks_bb<ROOK>(sq, Occupancy[2]);
    }

    while (bishop) {
      Square sq = pop_lsb(bishop);
      attacks |= attacks_bb<BISHOP>(sq, Occupancy[2]);
    }

    while (queen) {
      Square sq = pop_lsb(queen);
      attacks |= attacks_bb<BISHOP>(sq, Occupancy[2]) | attacks_bb<ROOK>(sq, Occupancy[2]);
    }

    Square ks = pop_lsb(king);
    attacks |= PsudoAttacks[KING][ks];

    if (by == WHITE)
      attacks |= pawn_attacks_bb<WHITE>(pawn);
    else
      attacks |= pawn_attacks_bb<BLACK>(pawn);

    return attacks & sqs;
  }



  void Board::moveRooks (Square to, int recover) {
    Square rook_from;
    Square rook_to;

    switch (to) {
      case g1 : rook_from = h1; rook_to = f1; break;
      case g8 : rook_from = h8; rook_to = f8; break;
      case c1 : rook_from = a1; rook_to = d1; break;
      case c8 : rook_from = a8; rook_to = d8; break;
      default : exit(-1);
    }

    if (recover == -1) {
      Piece rook = getPiece(rook_from);
      removePiece(rook, rook_from);
      setPiece(rook, rook_to);
    } else {
      Piece rook = getPiece(rook_to);
      removePiece(rook, rook_to);
      setPiece(rook, rook_from);
    }
  }
  void Board::capturedEP (Square to, int recover) {
    Direction d = turn == WHITE ? SOUTH : NORTH;
    Piece which = turn == WHITE ? B_PAWN : W_PAWN;

    if (recover == -1)
      removePiece(which, to + d);
    else
      setPiece(which, to + d);
  }
  void Board::removeRookCastle (Square from, Square to) {
    if (from == a1 || to == a1)
      CastleRights &= ~WHITE_OOO;
    if (from == h1 || to == h1)
      CastleRights &= ~WHITE_OO;
    if (from == a8 || to == a8)
      CastleRights &= ~BLACK_OOO;
    if (from == h8 || to == h8)
      CastleRights &= ~BLACK_OO;
  }


  void Board::MakeMove (const Move& move, State& state) {
    Square from = move.from_sq();
    Square to = move.to_sq();
    MoveType type = move.type_of();
    Piece p = getPiece(from);
    Piece pxx = getPiece(to);
    PieceType pt = type_of(p);
    Piece captured = NO_PIECE;

    //snapshot board
    uint8_t oldrights = CastleRights;
    state.key = key;
    state.CastleRights = CastleRights;
    state.Captured = captured;
    state.Enpassant = Enpassant;
    state.halfMove = halfMove;


    if (pxx != NO_PIECE) {
      captured = pxx;
      state.Captured = captured;
      removePiece(pxx, to);
      halfMove = 0;
    }

    if (pt == KING) {
      CastleRights &= turn == WHITE ? ~WHITE_CASTLING : ~BLACK_CASTLING;
    }

    removeRookCastle(from, to);

    if (type == PROMOTION) {
      PieceType promo = move.promotion_type();
      Piece promotion_piece = make_piece(turn, promo);

      removePiece(p, from);
      setPiece(promotion_piece, to);
      
    }
    else if (type == CASTLING) {
      removePiece(p, from);
      setPiece(p, to);
      moveRooks(to);
      CastlingRights remove = (turn == WHITE) ? WHITE_CASTLING : BLACK_CASTLING;
      CastleRights &= ~remove;
    }
    else if (type == EN_PASSANT) {
      removePiece(p, from);
      setPiece(p, to);
      capturedEP(to);
      halfMove = 0;
    }
    else {
      removePiece(p, from);
      setPiece(p, to);
    }

    if (Enpassant != Sq0)
      key ^= ephash(Enpassant);

    Enpassant = Sq0;
    if (pt == PAWN) {
      halfMove = 0;
      if (distance(from, to) == 2) {
        Enpassant = (turn == WHITE) ? from + NORTH : from + SOUTH;
      }
    }

    if (Enpassant != Sq0)
      key ^= ephash(Enpassant);

    if (oldrights != CastleRights) {

    }

    history.push_back(state);
    halfMove++;
    key ^= colorhash();
    turn = ~turn;

  }

  void Board::UnmakeMove (const Move& move) {
    State state = history.back();
    history.pop_back();

    Square from = move.from_sq();
    Square to = move.to_sq();
    MoveType type = move.type_of();

    turn = ~turn;

    Piece moved = getPiece(to);

    if (type == PROMOTION) {
      removePiece(moved, to);
      setPiece(make_piece(turn, PAWN), from);
    }
    else if (type == CASTLING) {
      removePiece(moved, to);
      setPiece(moved, from);
      moveRooks(to, 0);
    }
    else if (type == EN_PASSANT) {
      removePiece(moved, to);
      setPiece(moved, from);
      capturedEP(to, 0);
    }
    else {
      removePiece(moved, to);
      setPiece(moved, from);
    }

    if (state.Captured != NO_PIECE) {
      setPiece(state.Captured, to);
    }
    CastleRights = state.CastleRights;
    Enpassant = state.Enpassant;
    halfMove = state.halfMove;
    key = state.key;

  }

  void Board::MakeNullMove (State& state) {
    state.halfMove = halfMove;
    state.key = key;
    state.Enpassant = Enpassant;

    if (Enpassant != Sq0)
      key ^= ephash(Enpassant);
    Enpassant = Sq0;

    halfMove++;
    key ^= colorhash();
    turn = ~turn;
  }
  void Board::UnmakeNULLMove (State& state) {
    key = state.key;
    halfMove = state.halfMove;
    Enpassant = state.Enpassant;
    turn = ~turn;

  }




  Hash Board::compute_hash () {
    Hash h = 0;

    for (Square s = a1; s <= h8; ++s) {
      Piece p = getPiece(s);
      if (p != NO_PIECE)
        h ^= piecehash(p, s);
    }

    if (turn == BLACK)
      h ^= colorhash();

    h ^= castlehash(CastleRights);

    if (Enpassant != Sq0)
      h ^= ephash(Enpassant);

    return h;
  }
}
