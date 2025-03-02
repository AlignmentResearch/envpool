#include <iostream>
#include <string>

#include "envpool/sokoban/sokoban_node.h"

namespace sokoban {
// forward-declare RunAStar
void RunAStar(const std::string& level_file_name,
              const std::string& log_file_name, int total_levels_to_run = 1000,
              int fsa_limit = 1000000);
}  // namespace sokoban

int main(int argc, char** argv) {
  using namespace sokoban;
  int total_levels_to_run = 1000;
  int fsa_limit = 1000000;
  if (argc < 3) {
    std::cout
        << "Usage: " << argv[0]
        << " level_file_name log_file_name [total_levels_to_run] [fsa_limit]"
        << std::endl;
    return 1;
  }
  std::string level_file_name = argv[1];
  std::string log_file_name = argv[2];
  if (argc > 3) {
    total_levels_to_run = std::stoi(argv[3]);
  }
  if (argc > 4) {
    fsa_limit = std::stoi(argv[4]);
  }
  RunAStar(level_file_name, log_file_name, total_levels_to_run, fsa_limit);
  return 0;
}