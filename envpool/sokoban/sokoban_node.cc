#include "envpool/sokoban/sokoban_node.h"

namespace sokoban {

const std::vector<std::pair<int, int>> SokobanNode::delta = {
    {0, 1}, {1, 0}, {0, -1}, {-1, 0}  // Up, Right, Down, Left
};

bool SokobanNode::IsSameState(SokobanNode& rhs) {
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

void SokobanNode::PrintNodeInfo(std::vector<std::pair<int, int>>* goals) {
  for (int y = 0; y < dim_room; y++) {
    for (int x = 0; x < dim_room; x++) {
      bool is_wall = walls->at(x + y * dim_room);
      bool is_player = (x == player_x && y == player_y);
      bool is_box = false;
      bool is_goal = false;
      for (size_t i = 0; i < boxes.size(); i++) {
        if (boxes.at(i).first == x && boxes.at(i).second == y) {
          is_box = true;
          break;
        }
      }
      if (goals) {
        for (size_t i = 0; i < goals->size(); i++) {
          if (goals->at(i).first == x && goals->at(i).second == y) {
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

SokobanNode* SokobanNode::get_child_node(int delta_x, int delta_y) {
  int new_player_x = player_x + delta_x;
  int new_player_y = player_y + delta_y;
  // check if the move is valid
  if (check_wall(new_player_x, new_player_y)) {
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
      if (check_wall(new_box_x, new_box_y)) {
        return nullptr;
      }
      // check if the box is blocked by another box
      for (size_t j = 0; j < boxes.size(); j++) {
        if (boxes.at(j).first == new_box_x && boxes.at(j).second == new_box_y) {
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
  return new SokobanNode(dim_room, new_player_x, new_player_y, new_boxes, walls,
                         this);
}

bool SokobanNode::check_wall(int x, int y) {
  if (x < 0 || x >= dim_room || y < 0 || y >= dim_room) {
    return true;
  }
  return walls->at(x + y * dim_room);
}

size_t SokobanNode::Hash() const {
  size_t hash = 0;
  hash = (hash * 397) ^ std::hash<int>{}(player_x);
  hash = (hash * 397) ^ std::hash<int>{}(player_y);
  for (size_t i = 0; i < boxes.size(); i++) {
    hash = (hash * 397) ^ std::hash<int>{}(boxes.at(i).first);
    hash = (hash * 397) ^ std::hash<int>{}(boxes.at(i).second);
  }
  return hash;
}

bool SokobanNode::IsGoal(SokobanNode& goal_node) {
  for (size_t i = 0; i < boxes.size(); i++) {
    bool matched = false;
    for (size_t j = 0; j < goal_node.boxes.size(); j++) {
      if (boxes.at(i).first == goal_node.boxes.at(j).first &&
          boxes.at(i).second == goal_node.boxes.at(j).second) {
        matched = true;
        break;
      }
    }
    if (!matched) return false;
  }
  return true;
}

float SokobanNode::GoalDistanceEstimate(SokobanNode& goal_node) {
  float h = 0;
  for (size_t i = 0; i < boxes.size(); i++) {
    float min_distance = std::numeric_limits<float>::max();
    for (size_t j = 0; j < goal_node.boxes.size(); j++) {
      float distance = abs(boxes.at(i).first - goal_node.boxes.at(j).first) +
                       abs(boxes.at(i).second - goal_node.boxes.at(j).second);
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
    SokobanNode* new_node_ptr =
        get_child_node(delta.at(i).first, delta.at(i).second);
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

}  // namespace sokoban