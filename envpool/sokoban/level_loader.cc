#include "level_loader.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>


namespace sokoban {

size_t ERROR_SZ = 1024;

LevelLoader::LevelLoader(const std::filesystem::path& base_path)
    : levels(0), cur_level(levels.begin()), level_file_paths(0) {
  for (const auto& entry : std::filesystem::directory_iterator(base_path)) {
    level_file_paths.push_back(entry.path());
  }
}

void AddLine(SokobanLevel& level, const std::string& line) {
  if ((line.at(0) != '#') || (*line.rend() != '#')) {
    std::stringstream msg;
    msg << "Line '" << line
        << "' does not start and begin with '#', as it should." << std::endl;
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

void LevelLoader::LoadNewFile(std::mt19937& gen) {
  std::uniform_int_distribution<size_t> load_file_idx_r(
      0, level_file_paths.size() - 1);
  size_t load_file_idx = load_file_idx_r(gen);
  std::ifstream file(level_file_paths.at(load_file_idx));

  levels.clear();
  std::string line;
  while (std::getline(file, line)) {
    if (line.at(0) == '#') {
      SokobanLevel& cur_level = levels.emplace_back(0);
      cur_level.reserve(15 * 15);

      // Count contiguous '#' characters and use this as the box dimension
      size_t dim_room = 0;
      for (const char& r : line) {
        if (r == '#') {
          dim_room++;
        }
      }
      AddLine(cur_level, line);

      while (std::getline(file, line) && line.at(0) == '#') {
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
  std::shuffle(levels.begin(), levels.end(), gen);
  if(levels.empty()) {
      std::stringstream msg;
      msg << "No levels loaded from file '" << level_file_paths.at(load_file_idx) << std::endl;
      throw std::runtime_error(msg.str());
  }
}

const std::vector<SokobanLevel>::iterator LevelLoader::RandomLevel(std::mt19937& gen) {
  if (cur_level == levels.end()) {
    LoadNewFile(gen);
    cur_level = levels.begin();
    if(cur_level == levels.end()) {
        throw std::runtime_error("No levels loaded.");
    }
  }
  auto out = cur_level;
  cur_level++;
  return out;
}

}  // namespace sokoban
