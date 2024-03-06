#ifndef NODE_SOKOBAN_H_
#define NODE_SOKOBAN_H_

#include <memory>

#include "astar.h"
#include "envpool/sokoban/level_loader.h"

namespace sokoban {

class SokobanNode {
 public:
  static const std::vector<std::pair<int, int>> delta;
  int dim_room{0};
  int player_x{0}, player_y{0};
  std::vector<std::pair<int, int>> boxes;
  unsigned int total_boxes{0};
  std::shared_ptr<std::vector<bool>> walls;
  SokobanNode* parent_node{nullptr};

  SokobanNode() {}

  SokobanNode(int dim_room, const SokobanLevel& world, bool is_goal_node)
      : dim_room(dim_room),
        walls(std::make_shared<std::vector<bool>>(dim_room * dim_room, false)) {
    for (int y = 0; y < dim_room; y++) {
      for (int x = 0; x < dim_room; x++) {
        switch (world.at(x + y * dim_room)) {
          case PLAYER:
            player_x = x;
            player_y = y;
            break;
          case BOX:
            if (!is_goal_node) {
              total_boxes++;
              boxes.push_back(std::make_pair(x, y));
            }
            break;
          case TARGET:
            if (is_goal_node) {
              total_boxes++;
              boxes.push_back(std::make_pair(x, y));
            }
            break;
          case BOX_ON_TARGET:
            total_boxes++;
            boxes.push_back(std::make_pair(x, y));
            break;
          case PLAYER_ON_TARGET:
            player_x = x;
            player_y = y;
            break;
        }

        if (world.at(x + y * dim_room) == WALL) {
          walls->at(x + y * dim_room) = true;
        }
      }
    }
    assert(total_boxes == boxes.size());
  }

  SokobanNode(int dim_room, int player_x, int player_y,
              std::vector<std::pair<int, int>> boxes,
              std::shared_ptr<std::vector<bool>> walls,
              SokobanNode* parent_node = nullptr)
      : dim_room(dim_room),
        player_x(player_x),
        player_y(player_y),
        boxes(boxes),
        total_boxes(boxes.size()),
        walls(walls),
        parent_node(parent_node) {}

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

  void PrintNodeInfo(std::vector<std::pair<int, int>>* goals = nullptr);
};
}  // namespace sokoban

#endif  // NODE_SOKOBAN_H_
