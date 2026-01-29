#include "bitboard.h"
#include <iostream>

namespace Engine {
  
  uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

  Bitboard PsudoAttacks[PIECE_TYPE_NB][SQUARE_NB];
  Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];

  Magic Magics [SQUARE_NB][2];

  namespace {

    Bitboard RookTable [0x19000];
    Bitboard BishopTable [0x1480];

    void init_magics (PieceType pt, Bitboard table[], Magic Magics [][2]);

    Bitboard safe_distance (Square s, int steps) {
      Square to = Square(s + steps);
      return is_ok(to) && distance(s, to) <= 2 ? square_bb(to) : Bitboard(0);
    }
  }


  void Bitboards::init () {


    //SQdiS init
    for (Square s1 = a1; s1 <= h8; ++s1)
      for (Square s2 = a1; s2 <= h8; ++s2) {
        SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));
      }

    init_magics (BISHOP, BishopTable, Magics);
    init_magics (ROOK, RookTable, Magics);

    for (Square s1 = a1; s1 <= h8; ++s1) {

      PawnAttacks[WHITE][s1] = pawn_attacks_bb<WHITE>(square_bb(s1));
      PawnAttacks[BLACK][s1] = pawn_attacks_bb<BLACK>(square_bb(s1));

      for (int step : {-1, -7, -8, -9, 1, 7, 8, 9})
        PsudoAttacks[KING][s1] |= safe_distance(s1, step);

      for (int step : {-17, -15, -10, -6, 17, 15, 10, 6})
        PsudoAttacks[KNIGHT][s1] |= safe_distance(s1, step);

    }
  }

   namespace {

    Bitboard sliding_shots (PieceType pt, Square sq, Bitboard occupancy) {
      Bitboard attacks = 0;

      Direction RookDir[4] = {NORTH, SOUTH, EAST, WEST};
      Direction BishopDir[4] = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};

      for (Direction d : (pt == ROOK ? RookDir : BishopDir)) {
        Square s = sq;
        while (safe_distance(s, d)) {
          attacks |= (s += d);
          if (occupancy & s) break ;
        }
      }

      return attacks;
    }

    inline Bitboard rand64(std::mt19937_64& gen) {
      std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
      return dist(gen);
    }

    void init_magics (PieceType pt, Bitboard table[], Magic Magics[][2]) {

      Bitboard occupancy[4096];
      Bitboard refrence[4096];
      int epoch[4096] = {};
      int cnt = 0;
      int size = 0;

      for (Square sx = a1; sx <= h8; ++sx) {

        Bitboard edges = ( ((Rank1BB | Rank8BB) & ~rank_bb(sx)) | ((FileABB | FileHBB) & ~file_bb(sx)) );

        Magic& m = Magics[sx][pt - BISHOP];
        m.mask = sliding_shots(pt, sx, 0) & ~edges;
        m.shift = 64 - popcount(m.mask);
        m.attacks = sx == a1 ? table : Magics[sx - 1][pt - BISHOP].attacks + size;

        size = 0;
        Bitboard b = 0;

        do {
          occupancy[size] = b;
          refrence[size] = sliding_shots (pt, sx, b);
          size++;
          b = (b - m.mask) & m.mask;
        } while (b);

        std::mt19937_64 gen (std::random_device{}());
        Bitboard candidate;
        while (true) {
          candidate = rand64(gen) & rand64(gen) & rand64(gen);
          cnt++;
          bool fail = false;

          for (int i = 0; i < size; i++) {
            int index = (occupancy[i] * candidate) >> m.shift;
            if (epoch[index] < cnt) {
              epoch[index] = cnt;
              m.attacks[index] = refrence[i];
            }
            else if (m.attacks[index] != refrence[i]) {
              fail = true;
              break;
            }
          }
          if (!fail) break;
        }
        m.magic = candidate;

      }
    }
  }
}
