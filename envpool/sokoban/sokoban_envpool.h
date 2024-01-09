#ifndef ENVPOOL_SOKOBAN_H_
#define ENVPOOL_SOKOBAN_H_

#include <filesystem>

#include "envpool/core/async_envpool.h"
#include "envpool/core/env.h"
#include "envpool/core/array.h"

#include "level_loader.h"

namespace sokoban {

// class BaseSokobanEnvConfig(EnvConfig):
//     tinyworld_obs: bool = False
//     tinyworld_render: bool = False
//     max_episode_steps: int = 120  # default value from gym_sokoban
//     terminate_on_first_box: bool = False

//     reward_finished: float = 10.0  # Reward for completing a level
//     reward_box: float = 1.0  # Reward for putting a box on target
//     reward_step: float = -0.1  # Reward for completing a step
//
// class BoxobanConfig(BaseSokobanEnvConfig):

    // cache_path: Path = Path(__file__).parent.parent / ".sokoban_cache"
    // split: Literal["train", "valid", "test", None] = "train"
    // difficulty: Literal["unfiltered", "medium", "hard"] = "unfiltered"

class SokobanEnvFns {
 public:
  static decltype(auto) DefaultConfig() {
    return MakeDict("reward_finished"_.Bind(10.0f),
                    "reward_box"_.Bind(1.0f),
                    "reward_step"_.Bind(-0.1f),
                    "dim_room"_.Bind(10),
                    "levels_dir"_.Bind(std::string("None")));
  }
  template <typename Config>
  static decltype(auto) StateSpec(const Config& conf) {
    int dim_room = conf["dim_room"_];
    return MakeDict("obs"_.Bind(Spec<uint8_t>({3, dim_room, dim_room})));
  }
  template <typename Config>
  static decltype(auto) ActionSpec(const Config& conf) {
    return MakeDict("action"_.Bind(Spec<int>({-1}, {0, 8})));
  }
};

// this line will concat common config and common state/action spec
using SokobanEnvSpec = EnvSpec<SokobanEnvFns>;

class SokobanEnv : public Env<SokobanEnvSpec> {
  public:
        SokobanEnv(const Spec& spec, int env_id) : Env<SokobanEnvSpec>(spec, env_id), max_episode_steps{spec.config["max_episode_steps"_]},
            dim_room{static_cast<int>(spec.config["dim_room"_])},
            reward_finished{static_cast<float>(spec.config["reward_finished"_])},
            reward_box{static_cast<float>(spec.config["reward_box"_])},
            reward_step{static_cast<float>(spec.config["reward_step"_])},
            levels_dir{static_cast<std::string>(spec.config["levels_dir"_])},
            level_loader(levels_dir),
            internal_state_(WALL, static_cast<std::size_t>(dim_room*dim_room))
        {}

    bool IsDone () override { return done_; }
    void Reset() override;
    void Step(const Action &action) override;

    void WriteState();

  private:
    bool done_{true};
    int max_episode_steps, dim_room;
    float reward_finished, reward_box, reward_step;
    std::filesystem::path levels_dir;

    LevelLoader level_loader;
    SokobanLevel internal_state_;
    float _reward;
};

using SokobanEnvPool = AsyncEnvPool<SokobanEnv>;
}

#endif // ENVPOOL_SOKOBAN_H_
