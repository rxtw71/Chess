#include "hash.h"
#include "die.h"
#include "types.h"

namespace Engine {
  Hash pieceHashTable[12][64];
  Hash colorHashTable;
  Hash castleHashTable[16];
  Hash epHashTable[8];

  //additional helper fucntions dec
  int piece_toInt (Piece p) {
    switch (p) {
      case W_PAWN : return 0;
      case B_PAWN : return 1;
      case W_KNIGHT : return 2;
      case B_KNIGHT : return 3;
      case W_BISHOP : return 4;
      case B_BISHOP : return 5;
      case W_ROOK : return 6;
      case B_ROOK : return 7;
      case W_QUEEN : return 8;
      case B_QUEEN : return 9;
      case W_KING : return 10;
      case B_KING : return 11;
      default : die("Invalid hashing piece");
    }
    return-99;
  }

  inline Hash rand64(std::mt19937_64& gen) {
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    return dist(gen);
  }

  // main helpers
  Hash piecehash (Piece p, Square s) {
    return pieceHashTable [piece_toInt(p)][s];
  }
  Hash colorhash() {
    return colorHashTable;
  }
  Hash ephash(Square s) {
    return epHashTable[file_of(s)];
  } 
  Hash castlehash(uint8_t cr) {
    return castleHashTable[cr];
  }
  //initializers
  void init_hash () {
    std::mt19937_64 gen(0xCAFEBABE);

    for (int i = 0; i < 12; i++) {
      for (Square s = a1; s <= h8; ++s) {
        pieceHashTable[i][s] = rand64(gen);
      }
    }

    for (int i = 0; i < 16; i++) {
      castleHashTable[i] = rand64(gen);
    }

    for (int i = 0; i < 8; i++) {
      epHashTable[i] = rand64(gen);
    }

    colorHashTable = rand64(gen);

  }

}
