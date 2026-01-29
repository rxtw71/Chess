#include "bitboard.h"
#include "board.h"
#include "engine.h"
#include "movegen.h"
#include "hash.h"
#include "types.h"
#include "uci.h"

#include <iostream>

using namespace Engine;
std::string move_to_san(Board &b, Move m);

void gameloop (Board& b) {
  std::vector <std::string> pgn;
  std::cout << "Starting Game" << std::endl;
  while (true) {

    MoveList list;
    LegalMoves(b, list);

    if (DrawGame(b)) {
      std::cout << "GameDraw" << std::endl;
      break;
    }
    if (list.count == 0) {
      std::cout << "GameOver!!" << std::endl;
      break;
    }

    Board::State state;
    Move move = SearchMove(b, 15, 30000);

    pgn.push_back(move_to_san(b, move));
    b.MakeMove(move, state);
    b.print();
  }

  //prinitng pgn;
  int move_number = 1;
  for (int i = 0; i < pgn.size(); i++) {
    if (i % 2 == 0) {
      std::cout << move_number << ". ";
    }
    std::cout << pgn[i] << " ";
    if (i % 2 == 1)
      move_number++;
  }
  std::cout << std::endl;
}

int main() {
  Bitboards::init();
  init_hash();

  Board board;
  board.init();
  //board.loadFEN("4k3/8/8/8/8/8/8/3QK3 w - - 0 1");

  uci_loop();
}

std::string move_to_san(Board &b, Move m) {
  std::string san;
  char filechar[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

  if (m.type_of() == CASTLING) {
    Square to = m.to_sq();
    if (to == c1)
      return "O-O-O";
    else if (to == c8)
      return "o-o-o";
    else if (to == g1)
      return "O-O";
    else
      return "o-o";
  }

  Square from = m.from_sq();
  Square to = m.to_sq();
  Piece p = b.getPiece(from);
  Piece target = b.getPiece(to);

  if ((p == W_PAWN || p == B_PAWN) && target == NO_PIECE &&
      m.type_of() != PROMOTION && m.type_of() != EN_PASSANT)
    return squareStr(to);

  if (p != W_PAWN && p != B_PAWN) {
    switch (p) {
    case W_KNIGHT:
      san += "N";
      break;
    case B_KNIGHT:
      san += "N";
      break;
    case W_BISHOP:
      san += "B";
      break;
    case B_BISHOP:
      san += "B";
      break;
    case W_ROOK:
      san += "R";
      break;
    case B_ROOK:
      san += "R";
      break;
    case W_QUEEN:
      san += "Q";
      break;
    case B_QUEEN:
      san += "Q";
      break;
    case W_KING:
      san += "K";
      break;
    case B_KING:
      san += "K";
      break;
    default:
      san += "??";
    }
  }

  if (b.getPiece(to) != NO_PIECE || m.type_of() == EN_PASSANT) {
    if (p == W_PAWN || p == B_PAWN) {
      san += filechar[file_of(from)];
      san += 'x';
    } else {
      san += 'x';
    }
  }

  san += squareStr(to);

  // append promotion;
  if (m.type_of() == PROMOTION) {
    PieceType promotion = m.promotion_type();
    char charpp;
    switch (promotion) {
    case KNIGHT:
      charpp = 'N';
      break;
    case BISHOP:
      charpp = 'B';
      break;
    case ROOK:
      charpp = 'R';
      break;
    case QUEEN:
      charpp = 'Q';
      break;
    default:
      charpp = '?';
    }
    san += '=';
    san += charpp;
  }

  /*
  // append check, checkmatte
  Board::State st;
  b.MakeMove(m, st);
  if (b.kingInCheck()) {
    std::vector<Move> moves = LegalMoves(b, b.turn);
    if (moves.size() > 0) {
      san += '+';
    } else {
      san += '#';
    }
  }
  b.UnmakeMove(m);
  */

  return san;
}

