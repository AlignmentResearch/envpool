#ifndef LEVEL_LOADER_H_
#define LEVEL_LOADER_H_

#include <filesystem>
#include <random>
#include <vector>

namespace sokoban {

using SokobanLevel = std::vector<uint8_t>;

constexpr uint8_t WALL = 0;
constexpr uint8_t BOX = 4;
constexpr uint8_t PLAYER = 5;
constexpr uint8_t TARGET = 2;
constexpr uint8_t SPACE = 1;

class LevelLoader {
 protected:
  std::vector<SokobanLevel> levels;
  std::vector<SokobanLevel>::iterator cur_level;
  std::vector<std::filesystem::path> level_file_paths;
  void LoadNewFile(std::mt19937& gen);

 public:
  const std::vector<SokobanLevel>::iterator RandomLevel(std::mt19937& gen);
  LevelLoader(const std::filesystem::path& base_path);
};
}  // namespace sokoban

#endif  // LEVEL_LOADER_H_
