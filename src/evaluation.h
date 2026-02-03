#pragma once

#include "board.h"
#include "types.h"

namespace Leaf {

  int Eval (Board& b);
  int materialAdvantage (Board& b);
  int KnightEval (Board& b);
  int PawnEval (Board& b);

  //helper functions
  int GamePhase (Board&  b);
  double GameFactor (Board& b);
}

