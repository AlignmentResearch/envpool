#include "level_loader.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include "envpool/sokoban/utils.h"

namespace sokoban {

size_t ERROR_SZ = 1024;

LevelLoader::LevelLoader(const std::filesystem::path& base_path,
                         bool load_sequentially, int n_levels_to_load,
                         int verbose)
    : load_sequentially(load_sequentially),
      n_levels_to_load(n_levels_to_load),
      levels_loaded(0),
      levels(0),
      cur_level(levels.begin()),
      level_file_paths(0),
      verbose(verbose) {
  for (const auto& entry : std::filesystem::directory_iterator(base_path)) {
    level_file_paths.push_back(entry.path());
  }
  cur_file = level_file_paths.begin();
}

const std::string PRINT_LEVEL_KEY = "# .a$@s";

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

void LevelLoader::LoadFile(std::mt19937& gen) {
  std::filesystem::path file_path;
  if (load_sequentially) {
    if (cur_file == level_file_paths.end()) {
      throw std::runtime_error("No more files to load.");
    }
    file_path = *cur_file;
    cur_file++;
  } else {
    const size_t load_file_idx = safe_uniform_int(
        static_cast<size_t>(0), level_file_paths.size() - 1, gen);
    file_path = level_file_paths.at(load_file_idx);
  }
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
  if (!load_sequentially) {
    std::shuffle(levels.begin(), levels.end(), gen);
  }
  if (levels.empty()) {
    std::stringstream msg;
    msg << "No levels loaded from file '" << file_path << std::endl;
    throw std::runtime_error(msg.str());
  }

  if (verbose >= 1) {
    std::cout << "***Loaded " << levels.size() << " levels from " << file_path
              << std::endl;
    if (verbose >= 2) {
      PrintLevel(std::cout, levels.at(0));
      std::cout << std::endl;
      PrintLevel(std::cout, levels.at(1));
      std::cout << std::endl;
    }
  }
}

const std::vector<SokobanLevel>::iterator LevelLoader::GetLevel(
    std::mt19937& gen) {
  if (n_levels_to_load > 0 && levels_loaded >= n_levels_to_load) {
    throw std::runtime_error("Loaded all requested levels.");
  }
  if (cur_level == levels.end()) {
    LoadFile(gen);
    cur_level = levels.begin();
    if (cur_level == levels.end()) {
      throw std::runtime_error("No levels loaded.");
    }
  }
  auto out = cur_level;
  cur_level++;
  levels_loaded++;
  return out;
}

}  // namespace sokoban
