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

#include "level_loader.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sokoban {

LevelLoader::LevelLoader(const std::filesystem::path& base_path, int verbose)
    : levels_(0),
      cur_level_(levels_.begin()),
      level_file_paths_(0),
      verbose(verbose) {
  for (const auto& entry : std::filesystem::directory_iterator(base_path)) {
    level_file_paths_.push_back(entry.path());
  }
}

constexpr std::string kPrintLevelKey("# .a@$s");

void AddLine(SokobanLevel& level, const std::string& line) {
  auto start = line.at(0);
  auto end = line.at(line.size() - 1);
  if ((start != '#') || (start != '#')) {
    std::stringstream msg;
    msg << "Line '" << line << "' does not start (" << start << ") and end ("
        << end << ") with '#', as it should." << std::endl;
    throw std::runtime_error(msg.str());
  }
  for (const char& r : line) {
    switch (r) {
      case '#':
        level.push_back(kWall);
        break;
      case '@':
        level.push_back(kPlayer);
        break;
      case '$':
        level.push_back(kBox);
        break;
      case '.':
        level.push_back(kTarget);
        break;
      case ' ':
        level.push_back(kEmpty);
        break;
      default:
        std::stringstream msg;
        msg << "Line '" << line << "'has character '" << r
            << "' which is not in the valid set '#@$. '." << std::endl;
        throw std::runtime_error(msg.str());
        break;
    }
  }
}

void PrintLevel(std::ostream& os, const SokobanLevel& vec) {
  size_t dim_room = 0;
  for (; dim_room * dim_room != vec.size() && dim_room <= 100; dim_room++) {
  }  // take sqrt(vec.size())
  if (dim_room == 0) {
    throw std::runtime_error("dim_room cannot be zero.");
  }
  for (size_t i = 0; i < vec.size(); i++) {
    os << kPrintLevelKey.at(vec.at(i));
    if ((i + 1) % dim_room == 0) {
      os << std::endl;
    }
  }
}

void LevelLoader::LoadNewFile(std::mt19937& gen) {
  std::uniform_int_distribution<size_t> load_file_idx_r(
      0, level_file_paths_.size() - 1);
  const size_t load_file_idx = load_file_idx_r(gen);
  const std::filesystem::path& file_path = level_file_paths_.at(load_file_idx);
  std::ifstream file(file_path);

  levels_.clear();
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) {
      continue;
    }

    if (line.at(0) == '#') {
      SokobanLevel& cur_level = levels_.emplace_back(0);
      cur_level.reserve(10 * 10);  // In practice most levels are this size

      // Count contiguous '#' characters and use this as the box dimension
      size_t dim_room = 0;
      for (const char& r : line) {
        if (r == '#') {
          dim_room++;
        }
      }
      AddLine(cur_level, line);

      while (std::getline(file, line) && !line.empty() && line.at(0) == '#') {
        if (line.length() != dim_room) {
          std::stringstream msg;
          msg << "Irregular line '" << line
              << "' does not match dim_room=" << dim_room << std::endl;
          throw std::runtime_error(msg.str());
        }
        AddLine(cur_level, line);
      }

      if (cur_level.size() != dim_room * dim_room) {
        std::stringstream msg;
        msg << "Room is not square: " << cur_level.size() << " != " << dim_room
            << "x" << dim_room << std::endl;
        throw std::runtime_error(msg.str());
      }
    }
  }
  std::shuffle(levels_.begin(), levels_.end(), gen);
  if (levels_.empty()) {
    std::stringstream msg;
    msg << "No levels loaded from file '" << file_path << std::endl;
    throw std::runtime_error(msg.str());
  }

  if (verbose >= 1) {
    std::cout << "Loaded " << levels_.size() << " levels from " << file_path
              << std::endl;
    if (verbose >= 2) {
      PrintLevel(std::cout, levels_.at(0));
      std::cout << std::endl;
      PrintLevel(std::cout, levels_.at(1));
      std::cout << std::endl;
    }
  }
}

std::vector<SokobanLevel>::iterator LevelLoader::RandomLevel(
    std::mt19937& gen) {
  if (cur_level_ == levels_.end()) {
    LoadNewFile(gen);
    cur_level_ = levels_.begin();
    if (cur_level_ == levels_.end()) {
      throw std::runtime_error("No levels loaded.");
    }
  }
  auto out = cur_level_;
  cur_level_++;
  return out;
}

}  // namespace sokoban
