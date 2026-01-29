#pragma once

#include <cerrno>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <mutex>
#include <ostream>
#include <string>
#include <unistd.h>

//this file handles ouput for either tcp socket based or stdout based
//
namespace Engine {
  struct UCILog {
    std::ofstream log_file;
    std::mutex log_mutex; // optional for thread safety

    UCILog(const std::string& filename) {
      // std::ios::out automatically creates file if not present
      log_file.open(filename, std::ios::out);
      if (!log_file.is_open())
        std::cerr << "Failed to open UCI log file: " << filename << std::endl;
    }

    ~UCILog() {
      if (log_file.is_open())
        log_file.close();
    }

    void write(const std::string& s) {
      std::lock_guard<std::mutex> guard(log_mutex);
      if (log_file.is_open())
        log_file << s << std::endl; // flushes on each line
    }
  };

  struct Output {
    UCILog* log = nullptr;
    virtual void send (const std::string& s) = 0;
    virtual ~Output() {}
  };

  struct SocketO : Output {
    int fd;

    explicit SocketO(int fd_, UCILog* log_) : fd(fd_) {log = log_;}

    void send (const std::string& s) override {
      if (log)
        log->write("Client Output: " + s);
      std::string ss = s + "\n";
      ::write (fd, ss.c_str(), ss.size());
    }
  };
  struct CliO : Output {
    explicit CliO(UCILog* log_ = nullptr) {log = log_;}

    void send (const std::string& s) override {
      std::cout << s << std::endl;
      if (log)
        log->write("Terminal Output: " + s);
    }
  };

  struct Input {
    UCILog* log = nullptr;
    virtual bool listen (std::string& s) = 0;
    virtual ~Input() {}
  };
  struct SocketI : Input {
    int fd;
    std::string data;
    explicit SocketI(int fd_, UCILog* log_) : fd(fd_) {log = log_;}

    bool listen (std::string& s) override {
      char temp[256];
      int n = ::read(fd, temp, sizeof(temp));
      if (n <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
          return false;
        return false;
      }

      data.append(temp, n);
      auto pos = data.find('\n');
      if (pos == std::string::npos)
        return false;

      s = data.substr(0, pos);
      if (log)
        log->write("Client: " + s);
      data.erase(0, pos + 1);
      return true;
    }
  };
  struct CliI : Input {
    explicit CliI(UCILog* log_) {log = log_;}

    bool listen (std::string& s) override {
      if (!std::getline(std::cin, s))
        return false;
      if (log)
        log->write("Terminal: " + s);
      return true;
    }
  };


}

