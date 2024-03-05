#include "astar.h"
#include "envpool/sokoban/sokoban_envpool.h"

namespace sokoban {

const std::vector<std::pair<int, int>> SokobanNode::delta = {
    {0, 1}, {1, 0}, {0, -1}, {-1, 0}  // Up, Right, Down, Left
};

class SokobanNode {
 public:
  int dim_room{0};
  int player_x{0}, player_y{0};
  int total_boxes{0}, unmatched_boxes{0};
  std::vector<std::pair<int, int>> boxes;
  std::vector<std::pair<int, int>> goals;

  SokobanNode* parent_node;

  std::vector<bool>& walls;

  SokobanNode(int dim_room, const SokobanLevel& world)
      : dim_room(dim_room),
        walls(dim_room * dim_room, false),
        parent_node(nullptr) {
    for (int y = 0; y < dim_room; y++) {
      for (int x = 0; x < dim_room; x++) {
        switch (world.at(x + y * dim_room)) {
          case PLAYER:
            player_x = x;
            player_y = y;
            break;
          case BOX:
            total_boxes++;
            unmatched_boxes++;
            boxes.push_back(std::make_pair(x, y));
            break;
          case TARGET:
            goals.push_back(std::make_pair(x, y));
            break;
          case BOX_ON_TARGET:
            total_boxes++;
            boxes.push_back(std::make_pair(x, y));
            goals.push_back(std::make_pair(x, y));
            break;
          case PLAYER_ON_TARGET:
            player_x = x;
            player_y = y;
            goals.push_back(std::make_pair(x, y));
            break;
        }

        if (world.at(x + y * dim_room) == WALL) {
          walls.at(x + y * dim_room) = true;
        }
      }
    }
    assert(total_boxes == boxes.size());
    assert(total_boxes == goals.size());
  }

  // add another constructor to create a SokobanNode from player_x, player_y,
  // boxes, and goals
  SokobanNode(int dim_room, int player_x, int player_y,
              std::vector<std::pair<int, int>> boxes,
              std::vector<std::pair<int, int>> goals, std::vector<bool>& walls,
              SokobanNode* parent_node = nullptr)
      : dim_room(dim_room),
        player_x(player_x),
        player_y(player_y),
        boxes(boxes),
        goals(goals),
        walls(walls),
        total_boxes(boxes.size()),
        parent_node(parent_node) {
    unmatched_boxes = compute_unmatched_boxes();
  }

  int compute_unmatched_boxes() {
    unmatched_boxes = total_boxes;
    for (size_t i = 0; i < boxes.size(); i++) {
      for (size_t j = 0; j < goals.size(); j++) {
        if (boxes.at(i).first == goals.at(j).first &&
            boxes.at(i).second == goals.at(j).second) {
          unmatched_boxes--;
          break;
        }
      }
    }
    return unmatched_boxes;
  }

  bool check_wall(int x, int y) {
    if (x < 0 || x >= dim_room || y < 0 || y >= dim_room) {
      return true;
    }
    return walls.at(x + y * dim_room);
  }

  SokobanNode get_goal_node() {
    return SokobanNode(dim_room, player_x, player_y, goals, goals, walls);
  }

  SokobanNode get_child_node(int delta_x, int delta_y) {
    int new_player_x = player_x + delta_x;
    int new_player_y = player_y + delta_y;
    // check if the move is valid
    if (check_wall(new_player_x, new_player_y)) {
      return *this;
    }
    // check if (new_player_x, new_player_y) is a box, if it is not, return a
    // new SokobanNode with the new player position
    std::vector<std::pair<int, int>> new_boxes = boxes;
    for (size_t i = 0; i < boxes.size(); i++) {
      if (boxes.at(i).first == new_player_x &&
          boxes.at(i).second == new_player_y) {
        int new_box_x = boxes.at(i).first + delta_x;
        int new_box_y = boxes.at(i).second + delta_y;
        // check if the box can move
        if (check_wall(new_box_x, new_box_y)) {
          return *this;
        }
        // check if the box is blocked by another box
        for (size_t j = 0; j < boxes.size(); j++) {
          if (boxes.at(j).first == new_box_x &&
              boxes.at(j).second == new_box_y) {
            return *this;
          }
        }
        // update the box position
        new_boxes.at(i).first = new_box_x;
        new_boxes.at(i).second = new_box_y;
        break;
      }
    }
    return SokobanNode(new_player_x, new_player_y, new_boxes, goals, walls,
                       this);
  }

  float GoalDistanceEstimate();
  bool IsGoal();
  bool GetSuccessors(std::AStarSearch<SokobanNode>* astarsearch,
                     SokobanNode* parent_node);
  float GetCost(SokobanNode& successor);
  bool IsSameState(SokobanNode& rhs);
  size_t Hash() const;

  void PrintNodeInfo() {
    std::cout << "Player: (" << player_x << ", " << player_y << ")"
              << std::endl;
    std::cout << "Boxes: " << std::endl;
    for (size_t i = 0; i < boxes.size(); i++) {
      std::cout << "(" << boxes.at(i).first << ", " << boxes.at(i).second << ")"
                << std::endl;
    }
    std::cout << "Goals: " << std::endl;
    for (size_t i = 0; i < goals.size(); i++) {
      std::cout << "(" << goals.at(i).first << ", " << goals.at(i).second << ")"
                << std::endl;
    }
  }

}

bool SokobanNode::IsSameState(SokobanNode &rhs) {
  if (player_x != rhs.player_x || player_y != rhs.player_y) {
    return false;
  }
  for (size_t i = 0; i < boxes.size(); i++) {
    if (boxes.at(i).first != rhs.boxes.at(i).first ||
        boxes.at(i).second != rhs.boxes.at(i).second) {
      return false;
    }
  }
  return true;
}

size_t SokobanNode::Hash() const {
  size_t hash = 0;
  hash = (hash * 397) ^ std::hash<int>{}(player_x);
  hash = (hash * 397) ^ std::hash<int>{}(player_y);
  // hash should be the same regardless of the order of the boxes
  std::vector<std::pair<int, int>> sorted_boxes = boxes;
  std::sort(sorted_boxes.begin(), sorted_boxes.end());
  for (size_t i = 0; i < sorted_boxes.size(); i++) {
    hash = (hash * 397) ^ std::hash<int>{}(sorted_boxes.at(i).first);
    hash = (hash * 397) ^ std::hash<int>{}(sorted_boxes.at(i).second);
  }
  return hash;
}

bool SokobanNode::IsGoal() { return unmatched_boxes == 0; }

float SokobanNode::GoalDistanceEstimate() {
  float h = 0;
  for (size_t i = 0; i < boxes.size(); i++) {
    float min_distance = std::numeric_limits<float>::max();
    for (size_t j = 0; j < goals.size(); j++) {
      float distance = abs(boxes.at(i).first - goals.at(j).first) +
                       abs(boxes.at(i).second - goals.at(j).second);
      min_distance = std::min(min_distance, distance);
    }
    h += min_distance;
  }
  return h;
}

float SokobanNode::GetCost(SokobanNode& successor) { return 1; }

bool SokobanNode::GetSuccessors(std::AStarSearch<SokobanNode>* astarsearch,
                                SokobanNode* parent_node) {
  for (size_t i = 0; i < delta.size(); i++) {
    SokobanNode new_node =
        get_child_node(delta.at(i).first, delta.at(i).second);
    if (new_node == *this) {
      continue;
    }
    if (parent_node != nullptr && new_node == *parent_node) {
      continue;
    }
    astarsearch->AddSuccessor(new_node);
  }
}

int main(int argc, char* argv[]) {
  cout << "STL A* Search implementation\n(C)2001 Justin Heyes-Jones\n";

  // Create an instance of the search class...

  AStarSearch<SokobanNode> astarsearch;

  unsigned int SearchCount = 0;

  const unsigned int NumSearches = 1;

  while (SearchCount < NumSearches) {
    // Create a start state
    const std::string level_file = "sample_levels/000.txt" const int dim_room =
        10;

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

      cout << "Steps:" << SearchSteps << "\n";

      int len = 0;

      cout << "Open:\n";
      SokobanNode* p = astarsearch.GetOpenListStart();
      while (p) {
        len++;
#if !DEBUG_LIST_LENGTHS_ONLY
        ((SokobanNode*)p)->PrintNodeInfo();
#endif
        p = astarsearch.GetOpenListNext();
      }

      cout << "Open list has " << len << " nodes\n";

      len = 0;

      cout << "Closed:\n";
      p = astarsearch.GetClosedListStart();
      while (p) {
        len++;
#if !DEBUG_LIST_LENGTHS_ONLY
        p->PrintNodeInfo();
#endif
        p = astarsearch.GetClosedListNext();
      }

      cout << "Closed list has " << len << " nodes\n";
#endif

    } while (SearchState == AStarSearch<SokobanNode>::SEARCH_STATE_SEARCHING);

    if (SearchState == AStarSearch<SokobanNode>::SEARCH_STATE_SUCCEEDED) {
      cout << "Search found goal state\n";

      SokobanNode* node = astarsearch.GetSolutionStart();

#if DISPLAY_SOLUTION
      cout << "Displaying solution\n";
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

      cout << "Solution steps " << steps << endl;

      // Once you're done with the solution you can free the nodes up
      astarsearch.FreeSolutionNodes();

    } else if (SearchState == AStarSearch<SokobanNode>::SEARCH_STATE_FAILED) {
      cout << "Search terminated. Did not find goal state\n";
    }

    // Display the number of loops the search went through
    cout << "SearchSteps : " << SearchSteps << "\n";

    SearchCount++;

    astarsearch.EnsureMemoryFreed();
  }

  return 0;
}

}  // namespace sokoban