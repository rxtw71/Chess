#pragma once

#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "engine.h"
#include "types.h"
#include "board.h"
#include "perft.h"



namespace Leaf {

  //Input and output blue prints 
  struct Input {
    virtual bool listen(std::string& message) = 0;
    virtual ~Input() {}
  };
  struct Output {
    virtual void send(std::string message) = 0;
    virtual ~Output() {}
  };

  //Input methods
  struct TerminalI : Input {
    bool listen (std::string& message) override {
      if (!std::getline(std::cin, message))
        return false;
      return true;
    }
  };
  struct SocketI : Input {
    int fd;
    std::string data;
    explicit SocketI (int fd_) : fd(fd_) {}

    bool listen (std::string& message) override {
      char temp[256];
      int n = ::read(fd, temp, sizeof(temp));

      if ( n < 0)
        return false;

      data.append(temp, n);
      auto pos = data.find('\n');
      if (pos == std::string::npos)
        return false;

      message = data.substr(0, pos);
      data.erase(0, pos + 1);
      return true;
    }
  };

  //Output methods
  struct TerminalO : Output {
    void send(std::string message) override {
      std::cout << message << std::endl;
    }
  };
  struct SocketO : Output {
    int fd;
    explicit SocketO(int fd_) : fd(fd_) {}

    void send(std::string message) override {
      message += "\n";
      ::write(fd, message.c_str(), message.size());
    }
  };
/*------------------------struct for eaze of input and output---------------------*/

  void UCI_LOOP();
}
