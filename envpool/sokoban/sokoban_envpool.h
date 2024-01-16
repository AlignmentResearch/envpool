#ifndef ENVPOOL_SOKOBAN_H_
#define ENVPOOL_SOKOBAN_H_

#include <filesystem>

#include "envpool/core/array.h"
#include "envpool/core/async_envpool.h"
#include "envpool/core/env.h"
#include "level_loader.h"

namespace sokoban {

constexpr int ACT_NOOP = 0;
constexpr int ACT_PUSH_UP = 1;
constexpr int ACT_PUSH_DOWN = 2;
constexpr int ACT_PUSH_LEFT = 3;
constexpr int ACT_PUSH_RIGHT = 4;
constexpr int ACT_MOVE_UP = 5;
constexpr int ACT_MOVE_DOWN = 6;
constexpr int ACT_MOVE_LEFT = 7;
constexpr int ACT_MOVE_RIGHT = 8;
constexpr int MAX_ACTION = ACT_MOVE_RIGHT;

class SokobanEnvFns {
 public:
  static decltype(auto) DefaultConfig() {
    return MakeDict(
      "reward_finished"_.Bind(10.0),
      "reward_box"_.Bind(1.0),
      "reward_step"_.Bind(-0.1),
      "dim_room"_.Bind(10),
      "levels_dir"_.Bind(std::string("")),
      "verbose"_.Bind(0)
    );
  }
  template <typename Config>
  static decltype(auto) StateSpec(const Config& conf) {
    int dim_room = conf["dim_room"_];
    return MakeDict("obs"_.Bind(Spec<uint8_t>({3, dim_room, dim_room})));
  }
  template <typename Config>
  static decltype(auto) ActionSpec(const Config& conf) {
    return MakeDict("action"_.Bind(Spec<int>({-1}, {0, MAX_ACTION})));
  }
};

// this line will concat common config and common state/action spec
using SokobanEnvSpec = EnvSpec<SokobanEnvFns>;

class SokobanEnv : public Env<SokobanEnvSpec> {
 public:
  SokobanEnv(const Spec& spec, int env_id)
      : Env<SokobanEnvSpec>(spec, env_id),
        max_episode_steps{spec.config["max_episode_steps"_]},
        dim_room{static_cast<int>(spec.config["dim_room"_])},
        reward_finished{static_cast<double>(spec.config["reward_finished"_])},
        reward_box{static_cast<double>(spec.config["reward_box"_])},
        reward_step{static_cast<double>(spec.config["reward_step"_])},
        levels_dir{static_cast<std::string>(spec.config["levels_dir"_])},
        level_loader(levels_dir),
        world(WALL, static_cast<std::size_t>(dim_room * dim_room)),
        verbose(static_cast<int>(spec.config["verbose"_])) {}

  bool IsDone() override { return unmatched_boxes == 0; }
  void Reset() override;
  void Step(const Action& action) override;

  void WriteState(float reward);

 private:
  int max_episode_steps, dim_room;
  double reward_finished, reward_box, reward_step;
  std::filesystem::path levels_dir;

  LevelLoader level_loader;
  SokobanLevel world;
  int verbose;

  int player_x{0}, player_y{0};
  int unmatched_boxes{0};

  uint8_t WorldAt(int x, int y);
  void WorldAssignAt(int x, int y, uint8_t value);
};

using SokobanEnvPool = AsyncEnvPool<SokobanEnv>;
}  // namespace sokoban

#endif  // ENVPOOL_SOKOBAN_H_
