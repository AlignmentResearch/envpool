/*
 * Copyright 2023-2024 FAR AI
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ENVPOOL_SOKOBAN_SOKOBAN_ENVPOOL_H_
#define ENVPOOL_SOKOBAN_SOKOBAN_ENVPOOL_H_

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>

#include "envpool/core/array.h"
#include "envpool/core/async_envpool.h"
#include "envpool/core/env.h"
#include "level_loader.h"

namespace sokoban {

constexpr int kActNoop = 0;
constexpr int kActPushUp = 1;
constexpr int kActPushDown = 2;
constexpr int kActPushLeft = 3;
constexpr int kActPushRight = 4;
constexpr int kActMoveUp = 5;
constexpr int kActMoveDown = 6;
constexpr int kActMoveLeft = 7;
constexpr int kActMoveRight = 8;
constexpr int kMaxAction = kActMoveRight;

class SokobanEnvFns {
 public:
  static decltype(auto) DefaultConfig() {
    return MakeDict("reward_finished"_.Bind(10.0), "reward_box"_.Bind(1.0),
                    "reward_step"_.Bind(-0.1), "dim_room"_.Bind(10),
                    "levels_dir"_.Bind(std::string("")), "verbose"_.Bind(0),
                    "min_episode_steps"_.Bind(0),
                    "load_sequentially"_.Bind(false),
                    "n_levels_to_load"_.Bind(-1));
  }
  template <typename Config>
  static decltype(auto) StateSpec(const Config& conf) {
    int dim_room = conf["dim_room"_];
    return MakeDict("obs"_.Bind(Spec<uint8_t>({3, dim_room, dim_room})));
  }
  template <typename Config>
  static decltype(auto) ActionSpec(const Config& conf) {
    return MakeDict("action"_.Bind(Spec<int>({-1}, {0, kMaxAction})));
  }
};

// this line will concat common config and common state/action spec
using SokobanEnvSpec = EnvSpec<SokobanEnvFns>;

class SokobanEnv : public Env<SokobanEnvSpec> {
 public:
  SokobanEnv(const Spec& spec, int env_id)
      : Env<SokobanEnvSpec>(spec, env_id),
        dim_room_{static_cast<int>(spec.config["dim_room"_])},
        reward_finished_{static_cast<double>(spec.config["reward_finished"_])},
        reward_box_{static_cast<double>(spec.config["reward_box"_])},
        reward_step_{static_cast<double>(spec.config["reward_step"_])},
        levels_dir_{static_cast<std::string>(spec.config["levels_dir"_])},
        level_loader_(levels_dir_, spec.config["load_sequentially"_],
                      static_cast<int>(spec.config["n_levels_to_load"_]),
                      static_cast<int>(spec.config["verbose"_])),
        world_(kWall, static_cast<std::size_t>(dim_room_ * dim_room_)),
        verbose_(static_cast<int>(spec.config["verbose"_])),
        current_max_episode_steps_(
            static_cast<int>(spec.config["max_episode_steps"_])) {
    if (max_num_players_ != spec_.config["max_num_players"_]) {
      std::stringstream msg;
      msg << "max_num_players_ != spec_['max_num_players'] " << max_num_players_
          << " != " << spec_.config["max_num_players"_] << std::endl;
      throw std::runtime_error(msg.str());
    }

    if (max_num_players_ != spec.config["max_num_players"_]) {
      std::stringstream msg;
      msg << "max_num_players_ != spec['max_num_players'] " << max_num_players_
          << " != " << spec.config["max_num_players"_] << std::endl;
      throw std::runtime_error(msg.str());
    }
  }

  bool IsDone() override {
    return (unmatched_boxes_ == 0) ||
           (current_step_ >= current_max_episode_steps_);
  }
  void Reset() override;
  void Step(const Action& action_dict) override;

  void WriteState(float reward);

 private:
  int dim_room_;
  double reward_finished_, reward_box_, reward_step_;
  std::filesystem::path levels_dir_;

  LevelLoader level_loader_;
  SokobanLevel world_;
  int verbose_;

  int current_max_episode_steps_;
  int current_step_{0};
  int player_x_{0}, player_y_{0};
  int unmatched_boxes_{0};

  [[nodiscard]] uint8_t WorldAt(int x, int y) const;
  void WorldAssignAt(int x, int y, uint8_t value);
};

using SokobanEnvPool = AsyncEnvPool<SokobanEnv>;
}  // namespace sokoban

#endif  // ENVPOOL_SOKOBAN_SOKOBAN_ENVPOOL_H_
