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

void SokobanNode::PrintNodeInfo() {
  std::cout << "Player: (" << player_x << ", " << player_y << ")" << std::endl;
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
      break;
    }
  }
  return new SokobanNode(dim_room, new_player_x, new_player_y, new_boxes, goals,
                         walls, this);
}

SokobanNode SokobanNode::get_goal_node() {
  return SokobanNode(dim_room, player_x, player_y, goals, goals, walls);
}

bool SokobanNode::check_wall(int x, int y) {
  if (x < 0 || x >= dim_room || y < 0 || y >= dim_room) {
    return true;
  }
  return walls.at(x + y * dim_room);
}

int SokobanNode::compute_unmatched_boxes() {
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

bool SokobanNode::IsGoal(SokobanNode& goal_node) {
  return unmatched_boxes == 0;
}

float SokobanNode::GoalDistanceEstimate(SokobanNode& goal_node) {
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
}

}  // namespace sokoban