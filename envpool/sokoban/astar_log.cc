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

#include <fstream>
#include <sstream>

#include "envpool/sokoban/sokoban_node.h"

namespace sokoban {

void RunAStar(const std::string& file_idx, int fsa_limit = 1000000,
              const std::string& dataset = "train") {
  std::cout << "Running A* on file " << file_idx << " with fsa_limit "
            << fsa_limit << std::endl;
  std::stringstream filestream;
  filestream << "/training/.sokoban_cache/boxoban-levels-master/unfiltered/"
             << dataset << "/" << file_idx << ".txt";
  std::string level_file = filestream.str();
  const int dim_room = 10;
  const int total_levels = 1000;
  int level_idx = 0;
  LevelLoader level_loader(level_file, true, -1);
  std::mt19937 gen(42);
  std::stringstream logstream;
  logstream << "/training/.sokoban_cache/boxoban-levels-master/unfiltered/"
            << dataset << "/logs/"
            << "log_" << file_idx << ".csv";
  std::string log_file_name = logstream.str();

  std::ofstream log_file_out(log_file_name, std::ios_base::app);
  std::ifstream log_file_in(log_file_name);
  // check if the file is empty
  if (log_file_in.peek() == std::ifstream::traits_type::eof()) {
    log_file_out << "Level, Actions, Steps, SearchSteps" << std::endl;
  } else {  // skip levels that have already been run
    std::string line;
    std::getline(log_file_in, line);  // skip header
    while (std::getline(log_file_in, line)) {
      SokobanLevel level = *level_loader.GetLevel(gen);
      level_idx++;
    }
  }
  log_file_in.close();

  while (level_idx < total_levels) {
    std::AStarSearch<SokobanNode> astarsearch(fsa_limit);
    std::cout << "Running level " << level_idx << std::endl;
    SokobanLevel level = *level_loader.GetLevel(gen);

    SokobanNode node_start(dim_room, level, false);
    SokobanNode node_end(dim_room, level, true);
    astarsearch.SetStartAndGoalStates(node_start, node_end);
    unsigned int search_state;
    unsigned int search_steps = 0;
    std::cout << "Starting search" << std::endl;
    do {
      search_state = astarsearch.SearchStep();
      search_steps++;
    } while (search_state ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING);

    if (search_state == std::AStarSearch<SokobanNode>::SEARCH_STATE_SUCCEEDED) {
      std::stringstream loglinestream;
      loglinestream << level_idx << ", ";
      astarsearch.GetSolutionStart();
      int steps = 0;
      for (;;) {
        SokobanNode* node = astarsearch.GetSolutionNext();
        if (node == nullptr) {
          break;
        }
        int action = node->action_from_parent;
        assert(action >= 0 && action < 4);
        loglinestream << action;
        steps++;
      }
      loglinestream << ", " << steps << ", " << search_steps << std::endl;
      log_file_out << loglinestream.str();
      astarsearch.FreeSolutionNodes();
      astarsearch.EnsureMemoryFreed();
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_FAILED) {
      log_file_out << level_idx << ", "
                   << "SEARCH_STATE_FAILED, -1, " << search_steps << std::endl;
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_NOT_INITIALISED) {
      log_file_out << level_idx << ", "
                   << "SEARCH_STATE_NOT_INITIALISED, -1, " << search_steps
                   << std::endl;
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING) {
      log_file_out << level_idx << ", "
                   << "SEARCH_STATE_SEARCHING, -1, " << search_steps
                   << std::endl;
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_OUT_OF_MEMORY) {
      log_file_out << level_idx << ", "
                   << "SEARCH_STATE_OUT_OF_MEMORY, -1, " << search_steps
                   << std::endl;
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_INVALID) {
      log_file_out << level_idx << ", "
                   << "SEARCH_STATE_INVALID, -1, " << search_steps << std::endl;
    } else {
      log_file_out << level_idx << ", "
                   << "UNKNOWN, -1, " << search_steps << std::endl;
    }
    log_file_out.flush();
    level_idx++;
  }
}
}  // namespace sokoban

int main(int argc, char** argv) {
  std::string file_idx = "000";
  int fsa_limit = 1000000;
  std::string dataset = "train";
  if (argc > 1) {
    file_idx = argv[1];
  }
  if (argc > 2) {
    fsa_limit = std::stoi(argv[2]);
  }
  if (argc > 3) {
    dataset = argv[3];
  }
  sokoban::RunAStar(file_idx, fsa_limit, dataset);
  return 0;
}
