#pragma once

#include <string>
#include <iostream>

#include "engine.h"
#include "types.h"
#include "board.h"
#include "output.h"

namespace Engine {
  void UCI_LOOP(UCILog& logbook);
  void setup_socket(int port);
}
