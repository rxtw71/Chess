#pragma once

#include "types.h"
#include "bitboard.h"
#include "die.h"

namespace Leaf {
  using Hash = uint64_t;
  void init_hash ();

  Hash piecehash(Piece p, Square s);
  Hash castlehash(uint8_t cr);
  Hash colorhash();
  Hash ephash(Square s);
}
