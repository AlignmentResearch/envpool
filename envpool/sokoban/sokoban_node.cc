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

#include "envpool/sokoban/sokoban_node.h"

#include <algorithm>
#include <limits>

namespace sokoban {

bool SokobanNode::IsSameState(SokobanNode& rhs) const {
  if (player_x != rhs.player_x || player_y != rhs.player_y) {
    return false;
  }
  return boxes == rhs.boxes;
}

void SokobanNode::PrintNodeInfo(std::vector<std::pair<int, int>>* goals) {
  std::cout << "Action: " << action_from_parent << std::endl;
  for (int y = 0; y < dim_room; y++) {
    for (int x = 0; x < dim_room; x++) {
      bool is_wall = walls->at(x + y * dim_room);
      bool is_player = (x == player_x && y == player_y);
      bool is_box = false;
      bool is_goal = false;
      for (const auto& box : boxes) {
        if (box.first == x && box.second == y) {
          is_box = true;
          break;
        }
      }
      if (goals != nullptr) {
        for (const auto& goal : *goals) {
          if (goal.first == x && goal.second == y) {
            is_goal = true;
            break;
          }
        }
      }
      if (is_wall) {
        std::cout << "#";
      } else if (is_player) {
        if (is_goal) {
          std::cout << "a";
        } else {
          std::cout << "@";
        }
      } else if (is_box) {
        if (is_goal) {
          std::cout << "s";
        } else {
          std::cout << "$";
        }
      } else if (is_goal) {
        std::cout << ".";
      } else {
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
}

std::unique_ptr<SokobanNode> SokobanNode::GetChildNode(int action_idx) {
  int delta_x = kDelta.at(action_idx).at(0);
  int delta_y = kDelta.at(action_idx).at(1);
  int new_player_x = player_x + delta_x;
  int new_player_y = player_y + delta_y;
  // check if the move is valid
  if (CheckWall(new_player_x, new_player_y)) {
    return nullptr;
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
      if (CheckWall(new_box_x, new_box_y)) {
        return nullptr;
      }
      // check if the box is blocked by another box
      for (const auto& orig_box : boxes) {
        if (orig_box.first == new_box_x && orig_box.second == new_box_y) {
          return nullptr;
        }
      }
      // update the box position
      new_boxes.at(i).first = new_box_x;
      new_boxes.at(i).second = new_box_y;
      if (delta_y != 0) {
        std::sort(
            new_boxes.begin(), new_boxes.end(),
            [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
              if (a.second != b.second) {
                return a.second < b.second;
              }
              return a.first < b.first;
            });
      }
      break;
    }
  }
  return std::make_unique<SokobanNode>(dim_room, new_player_x, new_player_y,
                                       new_boxes, walls, this, action_idx);
}

bool SokobanNode::CheckWall(int x, int y) const {
  if (x < 0 || x >= dim_room || y < 0 || y >= dim_room) {
    return true;
  }
  return walls->at(x + y * dim_room);
}

size_t SokobanNode::Hash() const {
  size_t hash = 0;
  hash = (hash * 397) ^ std::hash<int>{}(player_x);
  hash = (hash * 397) ^ std::hash<int>{}(player_y);
  for (const auto& box : boxes) {
    hash = (hash * 397) ^ std::hash<int>{}(box.first);
    hash = (hash * 397) ^ std::hash<int>{}(box.second);
  }
  return hash;
}

bool SokobanNode::IsGoal(SokobanNode& goal_node) {
  for (const auto& box : boxes) {
    bool matched = false;
    for (const auto& goal_box : goal_node.boxes) {
      if (box == goal_box) {
        matched = true;
        break;
      }
    }
    if (!matched) {
      return false;
    }
  }
  return true;
}

float SokobanNode::GoalDistanceEstimate(SokobanNode& goal_node) {
  float h = 0;
  for (const auto& box : boxes) {
    float min_distance = std::numeric_limits<float>::max();
    bool goal_along_x = false;
    bool goal_along_y = false;
    for (const auto& goal_box : goal_node.boxes) {
      float distance =
          abs(box.first - goal_box.first) + abs(box.second - goal_box.second);
      min_distance = std::min(min_distance, distance);
      if (box.first == goal_box.first) {
        goal_along_y = true;
      }
      if (box.second == goal_box.second) {
        goal_along_x = true;
      }
    }
    h += min_distance;
    auto [surr_walls, contiguous_walls] = SurroundingWalls(box);
    if (contiguous_walls && min_distance != 0) {
      h += 1000;
    } else if (surr_walls == 1 && !goal_along_x && !goal_along_y) {
      h += 2;
    }
  }
  return h;
}

float SokobanNode::GetCost(SokobanNode& successor) { return 1; }

bool SokobanNode::GetSuccessors(std::AStarSearch<SokobanNode>* astarsearch,
                                SokobanNode* parent_node) {
  for (size_t i = 0; i < kDelta.size(); i++) {
    std::unique_ptr<SokobanNode> new_node_ptr = GetChildNode(i);
    if (new_node_ptr == nullptr) {
      continue;
    }
    if (parent_node != nullptr && new_node_ptr->IsSameState(*parent_node)) {
      continue;
    }
    astarsearch->AddSuccessor(*new_node_ptr);
  }
  return true;
}

std::pair<int, bool> SokobanNode::SurroundingWalls(
    const std::pair<int, int>& box) const {
  int num_walls = 0;
  bool found_wall = false;
  bool found_contiguous_wall = false;
  for (const auto& delta : kDelta) {
    int new_x = box.first + delta.at(0);
    int new_y = box.second + delta.at(1);
    if (CheckWall(new_x, new_y)) {
      num_walls++;
      if (found_wall) {
        found_contiguous_wall = true;
      }
      found_wall = true;
    } else {
      found_wall = false;
    }
  }
  if (found_wall) {
    int new_x = box.first + kDelta.at(0).at(0);
    int new_y = box.second + kDelta.at(0).at(1);
    found_contiguous_wall = found_contiguous_wall || CheckWall(new_x, new_y);
  }
  return std::make_pair(num_walls, found_contiguous_wall);
}

}  // namespace sokoban
