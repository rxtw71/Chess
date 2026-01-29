#pragma once

#include <string>
#include <iostream>

#include "engine.h"
#include "types.h"
#include "board.h"

namespace Engine {
  void uci_loop();
  void handle_position (Board& b, std::string line);
  void handle_go (Board& b, std::string line);
}
