#include "uci.h"
#include "board.h"
#include "engine.h"
#include "movegen.h"
#include "types.h"
#include <cstdio>
#include <sstream>
#include <string>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
namespace Engine {

  std::string moveUCI (Move move);
  void handle_position (Board& b, std::string line);
  void handle_go (Board& b, std::string line);
  void handle_go2 (Engine::Board& b, int& client);
  std::string unknown = "UNKNOW COMMAND: PLEASE ENTER A VALID COMMAND";


  static std::ofstream uciLog;
  void uci_loop() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // 127.0.0.1
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }

    listen(server_fd, 1);
    std::cerr << "UCI engine listening on port " << PORT << "\n";

    int client = accept(server_fd, nullptr, nullptr);
    if (client < 0) {
        perror("accept");
        return;
    }

    Board board;
    board.init();

    char buffer[4096];
    std::string line;
    uciLog.open("/storage/emulated/0/Download/uci.log", std::ios::out | std::ios::app);
    if (!uciLog.is_open()) {
      std::cerr << "Error" << std::endl;
    }

    std::string out;
    while (true) {
      ssize_t n = read(client, buffer, sizeof(buffer) - 1);
      if (n < 0)
        break;
      buffer[n] = 0;
      line += buffer;

      size_t pos;
      while ((pos = line.find('\n')) != std::string::npos) {

        std::string cmd = line.substr(0, pos);
        line.erase(0, pos + 1);

        if (uciLog.is_open()) {
          uciLog << "GUI -> " << cmd << std::endl;
          uciLog.flush();
        }

        //trime 
        //line.erase(0, line.find_first_not_of("\t"));

        //if (line.empty())
          //continue;

        if (cmd == "uci") {
          out = "id name Engine\r\n";
          write (client, out.c_str(), out.size());
          out =  "id auth rxtw71\r\n";
          write (client, out.c_str(), out.size());
          out = "uciok\r\n";
          write (client, out.c_str(), out.size());
        }

        else if (cmd == "ucinewgame") {
          board.init();
        }

        else if (cmd == "isready") {
          out = "readyok\r\n";
          std::cerr << out << std::endl;
          write (client, out.c_str(), out.size());
        }

        else if (cmd == "d")
          board.print();

        else if (cmd.rfind("position", 0) == 0)
          handle_position(board, cmd);

        else if (cmd.rfind("go", 0) == 0) {
          handle_go2(board, client);
          //handle_go(board, cmd);
        }

        else if (cmd =="quit")
          break;
      }
    }
  }
}
std::string Engine::moveUCI (Move move) {
  std::string squares = "";
  Square from = move.from_sq();
  Square to = move.to_sq();

  squares = squareStr(from) + squareStr(to);
  return squares;
}

std::string movetoUCI (Engine::Move move) {
  std::string movestr = "";

  movestr = Engine::squareStr(move.from_sq()) + Engine::squareStr(move.to_sq());
  if (move.type_of() == Engine::PROMOTION) {
    std::string p[] = {"n", "b", "r", "q"};
    Engine::PieceType pt = move.promotion_type();
    switch (pt) {
      case Engine::KNIGHT : movestr += p[0]; break;
      case Engine::BISHOP : movestr += p[1]; break;
      case Engine::ROOK : movestr += p[2]; break;
      case Engine::QUEEN : movestr += p[3]; break;
      default : printf("error");
    }
  }

  return movestr;
}

void Engine::handle_position (Engine::Board& b, std::string line) {
  std::vector <std::string> words;
  std::istringstream iss(line);
  std::string w;
  while (iss >> w)
    words.push_back(w);

  //handle startpos
  if (words[1] == "startpos") {
    b.init();
    Move m;

    for (int i = 2; i < words.size(); i++) {
      if (words[i] == "moves")
        continue;
      MoveList list;
      LegalMoves(b, list);
      for (int j = 0; j < list.count; j++) {
        if (moveUCI(list.data[j]) == words[i]) {
          m = list.data[j];
          break;
        }
      }
      Board::State st;
      b.MakeMove(m, st);
    }
  }
  else if (words[1] == "fen") {
    std::string fenstr = "";
    for (int i = 2; i <= 7; i++) {
      fenstr += words[i] + " ";
    }
    b.loadFEN(fenstr);
    Move m;

    for (int i = 8; i  < words.size(); i++) {
      if (words[i] == "moves")
        continue;
      MoveList list;
      LegalMoves(b, list);
      for (int j = 0; j < list.count; j++) {
        if (moveUCI(list.data[j]) == words[i]) {
          m =list.data[j];
          break;
        }
      }
      Board::State st;
      b.MakeMove(m, st);
    }

  }
  else {
    std::cout << unknown << std::endl;
  }
}
void Engine::handle_go (Engine::Board& b, std::string line) {
  std::istringstream iss(line);
  std::string word;
  int timeMs = 15000;
  while (iss >> word) {
    if (word == "movetime")
      iss >> timeMs;
  }
  Engine::Move best = Engine::SearchMove(b, 20, timeMs);
  std::cout << "bestmove " << moveUCI(best) << std::endl;

}
void Engine::handle_go2 (Engine::Board& b, int& client) {
  Engine::Move best = Engine::SearchMove(b, 20, 25000);
  std::string out = "bestmove " + moveUCI(best) + "\n";
  write (client, out.c_str(), out.size());
}
