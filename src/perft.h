#pragma once
#include "board.h"
#include "bitboard.h"
#include "movegen.h"

#include <vector>
#include <cstdint>
#include <iostream>
#include <chrono>

namespace Engine {

  inline std::string sqTostr (Square s) {
    char file = 'a' + (s % 8);
    char rank = '1' + (s / 8);
    return std::string() + file + rank;
  }

  inline  uint64_t perft(Board& b, int depth) {
    if (depth == 1) {
      MoveList list;
      LegalMoves(b, list);
      return list.count;
    }

    uint64_t nodes = 0;
    MoveList list;
    LegalMoves(b, list);  // generate pseudo-legal moves

    Board::State st;
    for (int i = 0; i < list.count; i++) {
      b.MakeMove(list.data[i], st);
      nodes += perft(b, depth-1);
      b.UnmakeMove(list.data[i]);
    }
    return nodes;
  }

  inline void perft_benchmark (Board& b, int depth) {
    auto start = std::chrono::steady_clock::now();
    std::cout << "Recieved board" << std::endl;
    uint64_t nodes = perft(b, depth);
    std::cout << "Perft done" << std::endl;
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    double nps = nodes / seconds;

    std::cout << "Perft (" << depth << ")" << std::endl;
    std::cout << "Nodes " << nodes << std::endl;
    std::cout << "Time " << seconds << std::endl;
    std::cout << "Nps " << nps << std::endl;
  }

  /*
  void perft_divide(Board& b, int depth) {
    assert(depth >= 1);

    uint64_t total = 0;

    std::vector<Move> moves = LegalMoves(b, b.turn);

    for (Move& m : moves) {
        Board back = b;
        Board::State st;

        back.MakeMove(m, st);

        uint64_t nodes = perft(back, depth - 1);

        back.UnmakeMove(m);

        std::cout << sqTostr(m.from_sq()) << " " << sqTostr(m.to_sq()) ;
        std::cout << ": " << nodes << "\n";

        total += nodes;
    }

    std::cout << "\nTotal nodes: " << total << "\n";
  }*/
}
