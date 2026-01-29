#include "uci.h"
#include "output.h"
#include <asm-generic/fcntl.h>
#include <cstddef>
#include <linux/in.h>
#include <sstream>
#include <string>
#include <cstring>
#include <sys/endian.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>

namespace Engine {

  int server_socket = -1;
  void setup_socket (int port) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    fcntl (server_socket, F_SETFL, O_NONBLOCK);
    //
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    //
    bind(server_socket, (struct sockaddr*)& addr, sizeof(addr));
    listen (server_socket, 1);

    std::cout << "Sever listening on port: " << port << std::endl;
  }

  //uci loopppppppppp
  void handle_command (std::string& line, int id, UCILog& logbook, int fd = -1);
  void UCI_LOOP (UCILog& logbook) {
    std::vector <Input*> inputs;
    CliI cListner(&logbook);
    inputs.push_back(&cListner);
    //
    SocketI* tListner = nullptr;


    // Cheap alt to avoid bloacking input of client;
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    std::string line;
    while (true) {

      //try to accept client
      if (!tListner && server_socket >= 0) {
        int tcp_fd = accept(server_socket, nullptr, nullptr);
        if (tcp_fd >= 0) {
          tListner = new SocketI(tcp_fd, &logbook);
          inputs.push_back(tListner);
          std::cout << "Client connected" << std::endl;
        }
      }

      if (tListner) {
        if (inputs[1]->listen(line)) {
          if (line == "quit")
            break;
          handle_command(line, 1, logbook, tListner->fd);
        }
      }
      else {
        if (inputs[0]->listen(line)) {
          if (line == "quit")
            break;
          handle_command(line, 0, logbook);
        }
      }

    }
    if (tListner) {
      close(tListner->fd);
      delete tListner;
    }
    if (server_socket >= 0)
      close(server_socket);
  }

  void handle_command (std::string& line, int id, UCILog& logbook, int fd) {
    Output* output = nullptr;
    if (id == 0)
      output = new CliO(&logbook);
    else
      output = new SocketO(fd, &logbook);

    std::vector<std::string> cmd;
    std::istringstream iss(line);
    std::string words;
    std::string op;
    while (iss >> words)
      cmd.push_back(words);

    if (cmd.empty())
      return;

    if (cmd[0] == "uci") {
      output->send("id name Engine");
      output->send("id auth rxtw71");
      output->send("uciok");
    }
    if (cmd[0] == "isready")
      output->send("readyok");



    delete output;
  }
}

