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

void RunAStar(const std::string& level_file_name,
              const std::string& log_file_name, int level_to_run = 0,
              int fsa_limit = 1000000) {
  std::cout << "Running A* on file " << level_file_name << " and logging to "
            << log_file_name << " with fsa_limit " << fsa_limit << "on level "
            << level_to_run << std::endl;
  const int dim_room = 10;
  int level_idx = 0;
  LevelLoader level_loader(level_file_name, true, -1);
  std::mt19937 gen(42);
  std::string file_idx =
      level_file_name.substr(level_file_name.find_last_of("/\\") + 1);
  file_idx = file_idx.substr(0, file_idx.find('.'));

  std::ofstream log_file_out(log_file_name, std::ios_base::app);

  while (level_idx < level_to_run) {
    level_loader.GetLevel(gen);
    level_idx++;
  }
  std::AStarSearch<SokobanNode> astarsearch(fsa_limit);
  std::cout << "Running level " << level_idx << std::endl;
  SokobanLevel level = *(level_loader.GetLevel(gen).first);

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
    loglinestream << file_idx << "," << level_idx << ",";
    SokobanNode* node = astarsearch.GetSolutionStart();
    int steps = 0;
    int prev_x = node->player_x;
    int prev_y = node->player_y;
    bool correct_solution = true;
    for (;;) {
      SokobanNode* node = astarsearch.GetSolutionNext();
      if (node == nullptr) {
        break;
      }
      int action = node->action_from_parent;
      assert(action >= 0 && action < 4);
      loglinestream << action;
      steps++;
      int curr_x = node->player_x;
      int curr_y = node->player_y;
      int delta_x = node->kDelta.at(action).at(0);
      int delta_y = node->kDelta.at(action).at(1);
      if (curr_x != prev_x + delta_x || curr_y != prev_y + delta_y) {
        correct_solution = false;
      }
      prev_x = curr_x;
      prev_y = curr_y;
    }
    if (!correct_solution) {
      loglinestream << ",INCORRECT_SOLUTION_FOUND," << search_steps
                    << std::endl;
    } else {
      loglinestream << "," << steps << "," << search_steps << std::endl;
    }
    log_file_out << loglinestream.str();
    astarsearch.FreeSolutionNodes();
    astarsearch.EnsureMemoryFreed();
  } else if (search_state ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_FAILED) {
    log_file_out << level_idx << "," << "SEARCH_STATE_FAILED,-1,"
                 << search_steps << std::endl;
  } else if (search_state ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_NOT_INITIALISED) {
    log_file_out << level_idx << "," << "SEARCH_STATE_NOT_INITIALISED,-1,"
                 << search_steps << std::endl;
  } else if (search_state ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING) {
    log_file_out << level_idx << "," << "SEARCH_STATE_SEARCHING,-1,"
                 << search_steps << std::endl;
  } else if (search_state ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_OUT_OF_MEMORY) {
    log_file_out << level_idx << "," << "SEARCH_STATE_OUT_OF_MEMORY,-1,"
                 << search_steps << std::endl;
  } else if (search_state ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_INVALID) {
    log_file_out << level_idx << "," << "SEARCH_STATE_INVALID,-1,"
                 << search_steps << std::endl;
  } else {
    log_file_out << level_idx << "," << "UNKNOWN,-1," << search_steps
                 << std::endl;
  }
  log_file_out.flush();
}
}  // namespace sokoban

int main(int argc, char** argv) {
  int fsa_limit = 1000000;
  if (argc < 4) {
    std::cout << "Usage: " << argv[0]
              << " level_file_name log_file_name level_to_run [fsa_limit]"
              << std::endl;
    return 1;
  }
  std::string level_file_name = argv[1];
  std::string log_file_name = argv[2];
  int level_to_run = std::stoi(argv[3]);
  if (argc > 4) {
    fsa_limit = std::stoi(argv[4]);
  }

  sokoban::RunAStar(level_file_name, log_file_name, level_to_run, fsa_limit);
  return 0;
}
