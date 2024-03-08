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

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "envpool/sokoban/sokoban_node.h"

#define DEBUG_LISTS 0
#define DEBUG_LIST_LENGTHS_ONLY 0

namespace sokoban {
TEST(SokobanAStarTest, Basic) {
  std::cout << "STL A* Search implementation\n(C)2001 Justin Heyes-Jones\n";

  // Create an instance of the search class...
  std::AStarSearch<SokobanNode> astarsearch(1000000);
  std::vector<int> verify_steps = {38, 19};
  std::vector<int> verify_search_steps = {67921, 26322};

  unsigned int search_count = 0;
  const unsigned int num_searches = 2;
  const std::string level_file = "/envpool/envpool/sokoban/sample_levels/";
  const int dim_room = 10;
  LevelLoader level_loader(level_file, false, 2);
  std::mt19937 gen(42);

  while (search_count < num_searches) {
    // Create a start state
    SokobanLevel level = *level_loader.GetLevel(gen);

    SokobanNode node_start(dim_room, level, false);
    SokobanNode node_end(dim_room, level, true);
    std::vector<std::pair<int, int>>* goals = &node_end.boxes;
    node_start.PrintNodeInfo(goals);
    astarsearch.SetStartAndGoalStates(node_start, node_end);

    unsigned int search_state;
    unsigned int search_steps = 0;

    do {
      search_state = astarsearch.SearchStep();

      search_steps++;

#if DEBUG_LISTS

      std::cout << "Steps:" << search_steps << "\n";

      int len = 0;

      std::cout << "Open:\n";
      SokobanNode* p = astarsearch.GetOpenListStart();
      while (p) {
        len++;
#if !DEBUG_LIST_LENGTHS_ONLY
        ((SokobanNode*)p)->PrintNodeInfo(goals);
#endif
        p = astarsearch.GetOpenListNext();
      }

      std::cout << "Open list has " << len << " nodes\n";

      len = 0;

      std::cout << "Closed:\n";
      p = astarsearch.GetClosedListStart();
      while (p) {
        len++;
#if !DEBUG_LIST_LENGTHS_ONLY
        p->PrintNodeInfo(goals);
#endif
        p = astarsearch.GetClosedListNext();
      }

      std::cout << "Closed list has " << len << " nodes\n";
#endif
    } while (search_state ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING);

    if (search_state == std::AStarSearch<SokobanNode>::SEARCH_STATE_SUCCEEDED) {
      std::cout << "Search found goal state\n";

      SokobanNode* node = astarsearch.GetSolutionStart();

      int steps = 0;

      node->PrintNodeInfo(goals);
      for (;;) {
        node = astarsearch.GetSolutionNext();

        if (node == nullptr) {
          break;
        }
        std::cout << "Step " << steps << std::endl;
        node->PrintNodeInfo(goals);
        steps++;
      }
      std::cout << "Solution steps " << steps << std::endl;
      EXPECT_EQ(steps, verify_steps.at(search_count));

      // Once you're done with the solution you can free the nodes up
      astarsearch.FreeSolutionNodes();

    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_FAILED) {
      std::cout << "Search terminated. Did not find goal state\n";
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_NOT_INITIALISED) {
      std::cout << "SEARCH_STATE_NOT_INITIALISED\n";
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING) {
      std::cout << "SEARCH_STATE_SEARCHING\n";
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_OUT_OF_MEMORY) {
      std::cout << "SEARCH_STATE_OUT_OF_MEMORY\n";
    } else if (search_state ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_INVALID) {
      std::cout << "SEARCH_STATE_INVALID\n";
    }

    // Display the number of loops the search went through
    std::cout << "search_steps : " << search_steps << "\n";
    EXPECT_EQ(search_state,
              std::AStarSearch<SokobanNode>::SEARCH_STATE_SUCCEEDED);
    EXPECT_EQ(search_steps, verify_search_steps.at(search_count));

    search_count++;

    astarsearch.EnsureMemoryFreed();
  }
}
}  // namespace sokoban
