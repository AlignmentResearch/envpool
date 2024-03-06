#ifndef NODE_SOKOBAN_H_
#define NODE_SOKOBAN_H_

#include "astar.h"
#include "envpool/sokoban/level_loader.h"

namespace sokoban {

class SokobanNode {
 public:
  static const std::vector<std::pair<int, int>> delta;
  int dim_room{0};
  int player_x{0}, player_y{0};
  unsigned int total_boxes{0}, unmatched_boxes{0};
  std::vector<std::pair<int, int>> boxes;
  std::vector<std::pair<int, int>> goals;

  SokobanNode* parent_node{nullptr};

  std::vector<bool> walls;

  SokobanNode() {}

  SokobanNode(int dim_room, const SokobanLevel& world) : dim_room(dim_room) {
    // initialize walls
    walls = std::vector<bool>(dim_room * dim_room, false);
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

  int compute_unmatched_boxes();

  bool check_wall(int x, int y);

  SokobanNode get_goal_node();

  SokobanNode* get_child_node(int delta_x, int delta_y);

  float GoalDistanceEstimate(SokobanNode& goal_node);
  bool IsGoal(SokobanNode& goal_node);
  bool GetSuccessors(std::AStarSearch<SokobanNode>* astarsearch,
                     SokobanNode* parent_node);
  float GetCost(SokobanNode& successor);
  bool IsSameState(SokobanNode& rhs);
  size_t Hash() const;

  void PrintNodeInfo();
};
}  // namespace sokoban

#endif  // NODE_SOKOBAN_H_
