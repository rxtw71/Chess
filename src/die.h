#pragma once
#include <iostream>
#include <cstdlib>
#include <string>



inline void die(const std::string& msg) {
    std::cerr << "ERROR: " << msg << std::endl;
    std::abort();  // stops the program immediately
}
