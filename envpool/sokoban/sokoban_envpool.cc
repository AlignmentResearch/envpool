#include "envpool/sokoban/sokoban_envpool.h"

#include <array>
#include <sstream>
#include <stdexcept>

#include "envpool/core/py_envpool.h"

namespace sokoban {

void SokobanEnv::Reset() {
  world = *level_loader.RandomLevel(gen_);
  if (world.size() != dim_room * dim_room) {
    std::stringstream msg;
    msg << "Loaded level is not dim_room x dim_room. world.size()="
        << world.size() << ", dim_room=" << dim_room << std::endl;
    throw std::runtime_error(msg.str());
  }
  unmatched_boxes = 0;
  for (int x = 0; x < dim_room; x++) {
    for (int y = 0; y < dim_room; y++) {
      switch (WorldAt(x, y)) {
        case PLAYER:
          player_x = x;
          player_y = y;
          break;
        case BOX:
          unmatched_boxes++;
          break;
      }
    }
  }
  WriteState(0.0f);
}

uint8_t SokobanEnv::WorldAt(int x, int y) {
  if ((x < 0) || (x >= dim_room) || (y < 0) || (y >= dim_room)) {
    return WALL;
  }
  return world.at(x + y * dim_room);
}
void SokobanEnv::WorldAssignAt(int x, int y, uint8_t value) {
  if ((x < 0) || (x >= dim_room) || (y < 0) || (y >= dim_room)) {
    return;
  }
  world.at(x + y * dim_room) = value;
}

constexpr std::array<std::array<int, 2>, 4> CHANGE_COORDINATES = {
    {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

void SokobanEnv::Step(const Action& action_) {
  const int action = action_["action"_];
  if (action == ACT_NOOP) {
    WriteState(static_cast<float>(reward_step));
    return;
  }
  // From here on, assume the agent will try to move

  const int change_coordinates_idx = (action - 1) % CHANGE_COORDINATES.size();
  const int delta_x = CHANGE_COORDINATES.at(change_coordinates_idx).at(0);
  const int delta_y = CHANGE_COORDINATES.at(change_coordinates_idx).at(1);

  const int prev_unmatched_boxes = unmatched_boxes;

  // Arena: the things that will change if the agent moves
  std::array<uint8_t, 3> arena;
  for (size_t i = 0; i < arena.size(); i++) {
    arena.at(i) = WorldAt(player_x + delta_x * i, player_y + delta_y * i);
  }

  // The box will move IFF action is a pushing action AND there's a box AND it
  // has space to move
  const bool box_moves =
      ((action <= ACT_PUSH_RIGHT) &&
       ((arena.at(1) == BOX) || (arena.at(1) == BOX_ON_TARGET)) &&
       ((arena.at(1) == EMPTY) || (arena.at(2) == TARGET)));

  // The agent will move if the next arena location is possible to move into, or
  // if it's a box and the box moves
  const bool is_a_box_and_the_box_moves = box_moves;
  const bool agent_moves = (arena.at(1) == EMPTY) || (arena.at(1) == TARGET) ||
                           is_a_box_and_the_box_moves;

  if (agent_moves) {
    // `is_target` is boolean but we'll need it as an int later
    std::array<int, arena.size()> is_target;
    for (size_t i = 0; i < arena.size(); i++) {
      uint8_t tile = arena.at(i);
      is_target.at(i) =
          (tile == BOX_ON_TARGET || tile == TARGET || tile == PLAYER_ON_TARGET);
    }
    // only whatever was on the floor is now at position 0
    arena.at(0) = is_target.at(0) ? TARGET : EMPTY;
    // the player now occupies position 1
    arena.at(1) = is_target.at(1) ? PLAYER_ON_TARGET : PLAYER;

    if (box_moves) {
      // the box moves for sure. A target at 2 reduces the nubmer of unmatched
      // boxes (because the box goes there), a target at 1 increases it (the box
      // leaves from there). Both can be equal to 1 and in that case the number
      // stays the same.
      unmatched_boxes += is_target.at(1) - is_target.at(2);

      // A box now occupies position 2
      arena.at(2) = is_target.at(2) ? BOX_ON_TARGET : BOX;
    }

    for (size_t i = 0; i < arena.size(); i++) {
      WorldAssignAt(player_x + delta_x * i, player_y + delta_y * i,
                    arena.at(i));
    }
    // After assigning the arena, move player.
    player_x += delta_x;
    player_y += delta_y;
  }

  const double reward =
      reward_step +
      reward_box * static_cast<double>(prev_unmatched_boxes - unmatched_boxes) +
      (IsDone() ? reward_finished : 0.0f);
  WriteState(static_cast<float>(reward));
}

constexpr std::array<std::array<uint8_t, 3>, PLAYER_ON_TARGET + 1> TINY_COLORS =
    {{
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
  if (obs.size != 3 * world.size()) {
    std::stringstream msg;
    msg << "Obs size and level size are different: obs_size=" << obs.size
        << "/3, level_size=" << world.size() << ", dim_room=" << dim_room
        << std::endl;
    throw std::runtime_error(msg.str());
  }

  std::vector<uint8_t> out(3 * world.size());
  for (int rgb = 0; rgb < 3; rgb++) {
    for (size_t i = 0; i < world.size(); i++) {
      out.at(rgb * (dim_room * dim_room) + i) =
          TINY_COLORS.at(world.at(i)).at(rgb);
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
