#include <glog/logging.h>
#include <gtest/gtest.h>

#include "envpool/sokoban/sokoban_node.h"

#define DEBUG_LISTS 0
#define DEBUG_LIST_LENGTHS_ONLY 0

namespace sokoban {
TEST(SokobanAStarTest, Basic) {
  std::cout << "STL A* Search implementation\n(C)2001 Justin Heyes-Jones\n";

  // Create an instance of the search class...

  std::AStarSearch<SokobanNode> astarsearch;

  unsigned int SearchCount = 0;

  const unsigned int NumSearches = 1;

  while (SearchCount < NumSearches) {
    // Create a start state
    const std::string level_file = "/envpool/envpool/sokoban/sample_levels/";
    const int dim_room = 10;

    LevelLoader level_loader(level_file);
    // make a rng for the level loader
    std::mt19937 gen(std::random_device{}());
    SokobanLevel level = *level_loader.RandomLevel(gen);

    SokobanNode nodeStart(dim_room, level);
    SokobanNode nodeEnd = nodeStart.get_goal_node();
    astarsearch.SetStartAndGoalStates(nodeStart, nodeEnd);

    unsigned int SearchState;
    unsigned int SearchSteps = 0;

    do {
      SearchState = astarsearch.SearchStep();

      SearchSteps++;

#if DEBUG_LISTS

      std::cout << "Steps:" << SearchSteps << "\n";

      int len = 0;

      std::cout << "Open:\n";
      SokobanNode* p = astarsearch.GetOpenListStart();
      while (p) {
        len++;
#if !DEBUG_LIST_LENGTHS_ONLY
        ((SokobanNode*)p)->PrintNodeInfo();
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
        p->PrintNodeInfo();
#endif
        p = astarsearch.GetClosedListNext();
      }

      std::cout << "Closed list has " << len << " nodes\n";
#endif

    } while (SearchState ==
             std::AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING);

    if (SearchState == std::AStarSearch<SokobanNode>::SEARCH_STATE_SUCCEEDED) {
      std::cout << "Search found goal state\n";

      SokobanNode* node = astarsearch.GetSolutionStart();

#if DISPLAY_SOLUTION
      std::cout << "Displaying solution\n";
#endif
      int steps = 0;

      node->PrintNodeInfo();
      for (;;) {
        node = astarsearch.GetSolutionNext();

        if (!node) {
          break;
        }

        node->PrintNodeInfo();
        steps++;
      };

      std::cout << "Solution steps " << steps << std::endl;

      // Once you're done with the solution you can free the nodes up
      astarsearch.FreeSolutionNodes();

    } else if (SearchState ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_FAILED) {
      std::cout << "Search terminated. Did not find goal state\n";
    } else if (SearchState ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_NOT_INITIALISED) {
      std::cout << "SEARCH_STATE_NOT_INITIALISED\n";
    } else if (SearchState ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING) {
      std::cout << "SEARCH_STATE_SEARCHING\n";
    } else if (SearchState ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_OUT_OF_MEMORY) {
      std::cout << "SEARCH_STATE_OUT_OF_MEMORY\n";
    } else if (SearchState ==
               std::AStarSearch<SokobanNode>::SEARCH_STATE_INVALID) {
      std::cout << "SEARCH_STATE_INVALID\n";
    }

    // Display the number of loops the search went through
    std::cout << "SearchSteps : " << SearchSteps << "\n";

    SearchCount++;

    astarsearch.EnsureMemoryFreed();
  }
}
}  // namespace sokoban