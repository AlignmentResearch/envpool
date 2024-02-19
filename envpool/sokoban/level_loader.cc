#include "level_loader.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace sokoban {

std::vector<std::filesystem::path> list_levels(
    const std::filesystem::path& base_path) {

  std::vector<std::filesystem::path> level_file_paths;
  for (const auto& entry : std::filesystem::directory_iterator(base_path)) {
    level_file_paths.push_back(entry.path());
  }
  return level_file_paths;
}

LevelLoader::LevelLoader(const std::filesystem::path& base_path,
                         LevelChoiceMode choice_mode, int verbose)
    : levels_(0),
      cur_level_(levels_.begin()),
      level_file_paths_(list_levels(base_path)),
      level_file_paths_iterator_(level_file_paths_.begin()),
      choice_mode_(choice_mode),
      verbose_(verbose) {}

const std::string PRINT_LEVEL_KEY = "# .a@$s";

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
        level.push_back(WALL);
        break;
      case '@':
        level.push_back(PLAYER);
        break;
      case '$':
        level.push_back(BOX);
        break;
      case '.':
        level.push_back(TARGET);
        break;
      case ' ':
        level.push_back(EMPTY);
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

void PrintLevel(std::ostream& os, SokobanLevel vec) {
  size_t dim_room = 0;
  for (; dim_room * dim_room != vec.size() && dim_room <= 100; dim_room++)
    ;  // take sqrt(vec.size())
  for (size_t i = 0; i < vec.size(); i++) {
    os << PRINT_LEVEL_KEY.at(vec.at(i));
    if ((i + 1) % dim_room == 0) {
      os << std::endl;
    }
  }
}

std::filesystem::path LevelLoader::ChooseRandomFile(std::mt19937& gen) {
  std::uniform_int_distribution<size_t> load_file_idx_r(
      0, level_file_paths.size() - 1);
  const size_t load_file_idx = load_file_idx_r(gen);
  return level_file_paths.at(load_file_idx);
}

void LevelLoader::LoadNewFile(const std::filesystem::path& file_path) {
  std::ifstream file(file_path);
  levels.clear();
  std::string line;
  while (std::getline(file, line)) {
    if (line.size() == 0) {
      continue;
    }

    if (line.at(0) == '#') {
      SokobanLevel& cur_level = levels.emplace_back(0);
      cur_level.reserve(10 * 10);  // In practice most levels are this size

      // Count contiguous '#' characters and use this as the box dimension
      size_t dim_room = 0;
      for (const char& r : line) {
        if (r == '#') {
          dim_room++;
        }
      }
      AddLine(cur_level, line);

      while (std::getline(file, line) && line.size() > 0 && line.at(0) == '#') {
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
  if (levels.empty()) {
    std::stringstream msg;
    msg << "No levels loaded from file '" << file_path << std::endl;
    throw std::runtime_error(msg.str());
  }

  if (verbose_ >= 1) {
    std::cout << "Loaded " << levels.size() << " levels from " << file_path
              << std::endl;
    if (verbose_ >= 2) {
      PrintLevel(std::cout, levels.at(0));
      std::cout << std::endl;
      PrintLevel(std::cout, levels.at(1));
      std::cout << std::endl;
    }
  }
}

const std::vector<SokobanLevel>::iterator LevelLoader::RandomLevel(
    std::mt19937& gen) {
  if (cur_level == levels.end()) {
    // Load levels from a file
    if (choice_mode_ == CHOICE_RANDOM) {
      auto random_file_path = ChooseRandomFile(gen);
      LoadNewFile(random_file_path);
      // Shuffle the levels after loading a random level
      std::shuffle(levels.begin(), levels.end(), gen);

    } else if (choice_mode_ == CHOICE_SEQUENTIAL) {
    } else {
      std::stringstream msg;
      msg << "Unknown choice_mode=" << choice_mode_ << std::endl;
      throw std::runtime_error(msg.str());
    }

    // set the next level to be the first loaded level
    cur_level = levels.begin();
    if (cur_level == levels.end()) {
      throw std::runtime_error("No levels loaded.");
    }
  }
  auto out = cur_level;
  cur_level++;
  return out;
}

}  // namespace sokoban
