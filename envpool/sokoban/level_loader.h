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

#ifndef ENVPOOL_SOKOBAN_LEVEL_LOADER_H_
#define ENVPOOL_SOKOBAN_LEVEL_LOADER_H_

#include <filesystem>
#include <random>
#include <vector>

namespace sokoban {

using SokobanLevel = std::vector<uint8_t>;

constexpr uint8_t kWall = 0;
constexpr uint8_t kEmpty = 1;
constexpr uint8_t kTarget = 2;
constexpr uint8_t kBoxOnTarget = 3;
constexpr uint8_t kBox = 4;
constexpr uint8_t kPlayer = 5;
constexpr uint8_t kPlayerOnTarget = 6;
constexpr uint8_t kMaxLevelObject = kPlayerOnTarget;

class LevelLoader {
 protected:
  std::vector<SokobanLevel> levels_;
  std::vector<SokobanLevel>::iterator cur_level_;
  std::vector<std::filesystem::path> level_file_paths_;
  void LoadNewFile(std::mt19937& gen);

 public:
  int verbose;

  std::vector<SokobanLevel>::iterator RandomLevel(std::mt19937& gen);
  explicit LevelLoader(const std::filesystem::path& base_path, int verbose = 0);
};

void PrintLevel(std::ostream& os, const SokobanLevel& vec);
}  // namespace sokoban

#endif  // ENVPOOL_SOKOBAN_LEVEL_LOADER_H_
