/*
 * Copyright 2023-2024 FAR AI Inc
 */
#include "envpool/sokoban/sokoban_envpool.h"

#include <array>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "envpool/core/py_envpool.h"

namespace sokoban {

void SokobanEnv::Reset() {
  const int max_episode_steps = spec_.config["max_episode_steps"_];
  const int min_episode_steps = spec_.config["min_episode_steps"_];
  std::uniform_int_distribution<int> episode_length_rand(min_episode_steps,
                                                         max_episode_steps);
  current_max_episode_steps_ = episode_length_rand(gen_);

  world_ = *(level_loader_.RandomLevel(gen_));
  if (world_.size() != dim_room_ * dim_room_) {
    std::stringstream msg;
    msg << "Loaded level is not dim_room x dim_room. world_.size()="
        << world_.size() << ", dim_room_=" << dim_room_ << std::endl;
    throw std::runtime_error(msg.str());
  }
  unmatched_boxes_ = 0;
  for (int x = 0; x < dim_room_; x++) {
    for (int y = 0; y < dim_room_; y++) {
      switch (WorldAt(x, y)) {
        case kPlayer:
          player_x_ = x;
          player_y_ = y;
          break;
        case kBox:
          unmatched_boxes_++;
          break;
      }
    }
  }
  current_step_ = 0;
  WriteState(0.0f);
}

[[nodiscard]] uint8_t SokobanEnv::WorldAt(int x, int y) const {
  if ((x < 0) || (x >= dim_room_) || (y < 0) || (y >= dim_room_)) {
    return kWall;
  }
  return world_.at(x + y * dim_room_);
}
void SokobanEnv::WorldAssignAt(int x, int y, uint8_t value) {
  if ((x < 0) || (x >= dim_room_) || (y < 0) || (y >= dim_room_)) {
    return;
  }
  world_.at(x + y * dim_room_) = value;
}

constexpr std::array<std::array<int, 2>, 4> kChangeCoordinates = {
    {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

void SokobanEnv::Step(const Action& action_dict) {
  current_step_++;

  const int action = action_dict["action"_];
  if (action == kActNoop) {
    WriteState(static_cast<float>(reward_step_));
    return;
  }
  // From here on, assume the agent will try to move

  const int change_coordinates_idx = (action - 1) % kChangeCoordinates.size();
  const int delta_x = kChangeCoordinates.at(change_coordinates_idx).at(0);
  const int delta_y = kChangeCoordinates.at(change_coordinates_idx).at(1);

  const int prev_unmatched_boxes = unmatched_boxes_;

  // Arena: the things that will change if the agent moves
  std::array<uint8_t, 3> arena;
  for (size_t i = 0; i < arena.size(); i++) {
    arena.at(i) = WorldAt(player_x_ + delta_x * i, player_y_ + delta_y * i);
  }

  // The box will move IFF action is a pushing action AND there's a box AND it
  // has space to move
  const bool box_moves =
      ((action <= kActPushRight) &&
       ((arena.at(1) == kBox) || (arena.at(1) == kBoxOnTarget)) &&
       ((arena.at(2) == kEmpty) || (arena.at(2) == kTarget)));

  // The agent will move if the next arena location is possible to move into, or
  // if it's a box and the box moves
  const bool is_a_box_and_the_box_moves = box_moves;
  const bool agent_moves = (arena.at(1) == kEmpty) ||
                           (arena.at(1) == kTarget) ||
                           is_a_box_and_the_box_moves;

  if (agent_moves) {
    std::array<bool, arena.size()> is_target;
    for (size_t i = 0; i < arena.size(); i++) {
      uint8_t tile = arena.at(i);
      is_target.at(i) =
          (tile == kBoxOnTarget || tile == kTarget || tile == kPlayerOnTarget);
    }
    // only whatever was on the floor is now at position 0
    arena.at(0) = is_target.at(0) ? kTarget : kEmpty;
    // the player now occupies position 1
    arena.at(1) = is_target.at(1) ? kPlayerOnTarget : kPlayer;

    if (box_moves) {
      // the box moves for sure. A target at 2 reduces the nubmer of unmatched
      // boxes (because the box goes there), a target at 1 increases it (the box
      // leaves from there). Both can be equal to 1 and in that case the number
      // stays the same.
      //
      // Implicit conversion from bool to int is always 0/1.
      // https://en.cppreference.com/w/cpp/language/implicit_conversion
      unmatched_boxes_ +=
          static_cast<int>(is_target.at(1)) - static_cast<int>(is_target.at(2));

      // A box now occupies position 2
      arena.at(2) = is_target.at(2) ? kBoxOnTarget : kBox;
    }

    for (size_t i = 0; i < arena.size(); i++) {
      WorldAssignAt(player_x_ + delta_x * i, player_y_ + delta_y * i,
                    arena.at(i));
    }
    // After assigning the arena, move player.
    player_x_ += delta_x;
    player_y_ += delta_y;
  }

  const double reward = reward_step_ +
                        reward_box_ * static_cast<double>(prev_unmatched_boxes -
                                                          unmatched_boxes_) +
                        ((unmatched_boxes_ == 0) ? reward_finished_ : 0.0f);
  WriteState(static_cast<float>(reward));
}

constexpr std::array<std::array<uint8_t, 3>, kPlayerOnTarget + 1> kTinyColors{{
    {0, 0, 0},        // WALL
    {243, 248, 238},  // EMPTY
    {254, 126, 125},  // TARGET
    {254, 95, 56},    // BOX_ON_TARGET
    {142, 121, 56},   // BOX
    {160, 212, 56},   // PLAYER
    {219, 212, 56}    // PLAYER_ON_TARGET
}};

void SokobanEnv::WriteState(float reward) {
  auto state = Allocate();
  state["reward"_] = reward;
  Array& obs = state["obs"_];
  if (obs.size != 3 * world_.size()) {
    std::stringstream msg;
    msg << "Obs size and level size are different: obs_size=" << obs.size
        << "/3, level_size=" << world_.size() << ", dim_room=" << dim_room_
        << std::endl;
    throw std::runtime_error(msg.str());
  }

  std::vector<uint8_t> out(3 * world_.size());
  for (int rgb = 0; rgb < 3; rgb++) {
    for (size_t i = 0; i < world_.size(); i++) {
      out.at(rgb * (dim_room_ * dim_room_) + i) =
          kTinyColors.at(world_.at(i)).at(rgb);
    }
  }
  obs.Assign(out.data(), out.size());
}

}  // namespace sokoban

// generate python-side (raw) SokobanEnvSpec
using SokobanEnvSpec = PyEnvSpec<sokoban::SokobanEnvSpec>;
// generate python-side (raw) SokobanEnvPool
using SokobanEnvPool = PyEnvPool<sokoban::SokobanEnvPool>;

// generate sokoban_envpool.so
PYBIND11_MODULE(sokoban_envpool, m) {
  REGISTER(m, SokobanEnvSpec, SokobanEnvPool)
}
