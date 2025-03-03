// Copyright 2023-2024 FAR AI
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
  int total_levels_to_run = 1000;
  int fsa_limit = 1000000;
  if (argc < 3) {
    std::cout
        << "Usage: " << argv[0]
        << " level_file_name log_file_name [total_levels_to_run] [fsa_limit]\n";
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
  sokoban::RunAStar(level_file_name, log_file_name, total_levels_to_run,
                    fsa_limit);
  return 0;
}
