#include "auth.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace deg {
namespace auth {

std::string read_str_from_file(std::string filename) {
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

} // namespace auth
} // namespace deg