#include "uci.h"
#include "engine.h"
#include "movegen.h"
#include "perft.h"
#include <atomic>
#include <cstddef>
#include <functional>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>


namespace Leaf {

  Board board;
  //Gloabal flags
  std::atomic<bool> clientConnected(false);
  std::atomic<bool> acceptClient(true);
  std::atomic<int> client_fd(-1);
  std::atomic<bool> isRunning(true);
  int SERVER_FD;

  //Forward Dec..
  void acceptingClient (int SERVER_FD);
  int startServer (int PORT);
  void listenTerminal ();
  void listenClient ();
  void handleCommand (Output& out, std::string cmd);
  std::string moveToString(const Move& m);
  std::string pvToStr();
  Move stringToMove (std::string m);
  void goMove (Output& out, int depth);
  void goPerft (Output& out, int depth);



  void UCI_LOOP () {
    SERVER_FD = startServer(5000);
    board.init();

    std::thread acceptThread(acceptingClient, SERVER_FD);
    std::thread listenThread1(listenTerminal);

    acceptThread.detach();
    listenThread1.join();
    close(SERVER_FD);
  }

  int startServer (int PORT) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
      die("socket:");

    sockaddr_in addr{};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if (bind(fd, (sockaddr*)& addr, sizeof(addr)) < 0)
      die("bind");

    if (listen(fd, 1) < 0)
      die("listen");

    std::cout << "Server listening on " << PORT << "..." << std::endl;
    return fd;
  }

  void acceptingClient (int SERVER_FD) {
    while (acceptClient.load()) {
      int fd = accept(SERVER_FD, nullptr, nullptr);
      if (fd < 0)
        continue;

      if (clientConnected.load()) {
        close(fd);
        continue;
      }

      client_fd.store(fd);
      clientConnected.store(true);
      acceptClient.store(false);
      std::thread listenThread2(listenClient);
      listenThread2.detach();
      std::cout << "Client Connected" << std::endl;
      return;
    }
  }

  void listenTerminal () {
    TerminalI ti;
    TerminalO to;
    std::string cmd;
    while (isRunning.load()) {
      if (!ti.listen(cmd)) break;
      if (!isRunning.load()) break;
      handleCommand(std::ref(to), cmd);
    }
  }

  void listenClient () {
    SocketI si(client_fd.load());
    SocketO so(client_fd.load());
    std::string cmd;
    while (isRunning) {
      if (!si.listen(cmd)) break;
      handleCommand(std::ref(so), cmd);
    }
    close(client_fd.load());
    client_fd.store(-1);
    clientConnected.store(false);
    acceptClient.store(true);
    std::cout << "Client Disconnected." << std::endl;
  }

  std::vector <std::string> split (std::string& line) {
    std::istringstream iss(line);
    std::vector <std::string> tokens;
    std::string words;
    while (iss >> words) {
      tokens.push_back(words);
    }
    return tokens;
  }

  std::string UNKNOWN = "Unknown command.";
  void handleCommand (Output& out, std::string cmd) {
    auto words = split(cmd);
    //command phraser
    if (words[0] =="quit") {
      isRunning.store(false);
      acceptClient.store(false);
      return;
    }
    else if (words[0] == "uci") {
      out.send("id name Leaf");
      out.send("uciok");
      return;
    }
    else if (words[0] == "isready") {
      out.send("readyok");
      return;
    }
    else if (words[0] == "stop") {
      stopSearch.store(true);
      return;
    }
    else if (words[0] == "d" || words[0] == "board") {
      board.print();
    }
    else if (words[0] == "go") {


      if (words[1] == "depth") {
        int depth = std::stoi(words[2]);
        std::thread searchThread(goMove, std::ref(out), depth);
        searchThread.detach();
      }
      else if (words[1] == "perft") {
        int depth = std::stoi(words[2]);
        std::thread perftThread(goPerft, std::ref(out), depth);
        perftThread.detach();
      }
      else {
        out.send(UNKNOWN);
      }

    }
    else if (words[0] == "position") {

      if (words[1] == "startpos") {
        board.init();

        for (int i = 2; i < words.size(); i++) {
          if (words[i] == "moves")
            continue;
          Move move = stringToMove(words[i]);
          Board::State state;
          board.MakeMove(move, state);
        }
      }
      else if (words[1] == "fen") {

        std::string fen;

        for (int i = 2; i < 8; i++) {
          fen += words[i] + " ";
        }
        board.loadFEN(fen);

        for (int i = 8; i < words.size(); i++) {
          if (words[i] == "moves")
            continue;
          Move move = stringToMove(words[i]);
          Board::State state;
          board.MakeMove(move, state);
        }
      }
      else if (words[1] == "move") {
        for (int i = 2; i < words.size(); i++) {
          if (words[i] == "moves")
            continue;
          Move move = stringToMove(words[i]);
          Board::State state;
          board.MakeMove(move, state);
        }
      }
      else {
        out.send(UNKNOWN);
      }

    }
    else {
      out.send(UNKNOWN);
    }
  }

  void goMove (Output& out, int depth) {
    Move m = SearchMove(board, depth);
    std::string output = "bestmove ";
    output += moveToString(m);
    out.send(pvToStr());
    out.send(output);
  }
  void goPerft (Output& out, int depth) {
    int nodes = perft(board, depth);
    out.send("Nodes searched: " + std::to_string(nodes));
  }

}


namespace Leaf {

std::string moveToString(const Move& m) {
    // Convert square index to file+rank like e2, h7 etc.
    auto squareToStr = [](Square s) -> std::string {
        char file = 'a' + (s % 8);       // 0->a, 1->b ... 7->h
        char rank = '1' + (s / 8);       // 0->1, 8->2 ... 63->8
        return std::string() + file + rank;
    };

    std::string uci;
    uci += squareToStr(m.from_sq());
    uci += squareToStr(m.to_sq());

    // Add promotion piece if needed
    if (m.type_of() == PROMOTION) {
        auto promo = m.promotion_type();  // assuming you have this function
        char pChar = 'q';              // default queen
        switch (promo) {
            case KNIGHT: pChar = 'n'; break;
            case BISHOP: pChar = 'b'; break;
            case ROOK:   pChar = 'r'; break;
            case QUEEN:  pChar = 'q'; break;
            default: break;
        }
        uci += pChar;
    }

    return uci;
}

Move stringToMove (std::string m) {
  int ff = m[0] - 'a';
  int fr = m[1] - '1';
  int tf = m[2] - 'a';
  int tr = m[3] - '1';
  Square f = Square(fr * 8 + ff);
  Square t = Square(tr * 8 + tf);
  Move move = Move::none();

  MoveList list;
  LegalMoves(board, list);
  for (int i = 0; i < list.count; i++) {
    if (f == list.data[i].from_sq() && t == list.data[i].to_sq()) {
      if (m.size() != 5)
        return list.data[i];

      char p = m[4];
      PieceType pt = (p == 'q') ? QUEEN : (p == 'r') ? ROOK : (p == 'b') ? BISHOP : KNIGHT;
      if (list.data[i].promotion_type() == pt)
        return list.data[i];
    }
  }
  return move;
}

std::string pvToStr() {
  std::string s = "PV: ";
  auto v = readPV();
  for (auto u : v) {
    s += moveToString(u) + " ";
  }
  return s;
}

}
