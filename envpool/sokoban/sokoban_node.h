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

#ifndef ENVPOOL_SOKOBAN_SOKOBAN_NODE_H_
#define ENVPOOL_SOKOBAN_SOKOBAN_NODE_H_

#include <memory>
#include <utility>
#include <vector>

#include "astar.h"
#include "envpool/sokoban/level_loader.h"

namespace sokoban {

class SokobanNode {
 public:
  static const std::vector<std::pair<int, int>> kDelta;
  int dim_room{0};
  int player_x{0}, player_y{0};
  std::vector<std::pair<int, int>> boxes;
  unsigned int total_boxes{0};
  std::shared_ptr<std::vector<bool>> walls;
  SokobanNode* parent_node{nullptr};
  int action_from_parent{-1};  // -1 is for when node is root
  bool is_goal_node{false};

  SokobanNode() = default;

  SokobanNode(int dim_room, const SokobanLevel& world, bool is_goal_node)
      : dim_room(dim_room),
        walls(std::make_shared<std::vector<bool>>(dim_room * dim_room, false)),
        is_goal_node(is_goal_node) {
    for (int y = 0; y < dim_room; y++) {
      for (int x = 0; x < dim_room; x++) {
        switch (world.at(x + y * dim_room)) {
          case kPlayer:
            player_x = x;
            player_y = y;
            break;
          case kBox:
            if (!is_goal_node) {
              total_boxes++;
              boxes.emplace_back(std::make_pair(x, y));
            }
            break;
          case kTarget:
            if (is_goal_node) {
              total_boxes++;
              boxes.emplace_back(std::make_pair(x, y));
            }
            break;
          case kBoxOnTarget:
            total_boxes++;
            boxes.emplace_back(std::make_pair(x, y));
            break;
          case kPlayerOnTarget:
            player_x = x;
            player_y = y;
            break;
        }

        if (world.at(x + y * dim_room) == kWall) {
          walls->at(x + y * dim_room) = true;
        }
      }
    }
    assert(total_boxes == boxes.size());
  }

  SokobanNode(int dim_room, int player_x, int player_y,
              std::vector<std::pair<int, int>> boxes,
              std::shared_ptr<std::vector<bool>> walls,
              SokobanNode* parent_node = nullptr, int action_from_parent = -1)
      : dim_room(dim_room),
        player_x(player_x),
        player_y(player_y),
        boxes(boxes),
        total_boxes(boxes.size()),
        walls(walls),
        parent_node(parent_node),
        action_from_parent(action_from_parent) {}

  void UpdateGoalNode(SokobanNode goal_node) {
    assert(goal_node.is_goal_node && is_goal_node);
    player_x = goal_node.player_x;
    player_y = goal_node.player_y;
    parent_node = goal_node.parent_node;
    action_from_parent = goal_node.action_from_parent;
  }

  bool CheckWall(int x, int y);

  SokobanNode* GetChildNode(int action_idx);

  float GoalDistanceEstimate(SokobanNode& goal_node);
  bool IsGoal(SokobanNode& goal_node);
  bool GetSuccessors(std::AStarSearch<SokobanNode>* astarsearch,
                     SokobanNode* parent_node);
  float GetCost(SokobanNode& successor);
  bool IsSameState(SokobanNode& rhs);
  [[nodiscard]] size_t Hash() const;

  void PrintNodeInfo(std::vector<std::pair<int, int>>* goals = nullptr);
};
}  // namespace sokoban

#endif  // ENVPOOL_SOKOBAN_SOKOBAN_NODE_H_
