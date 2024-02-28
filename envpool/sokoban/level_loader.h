#ifndef LEVEL_LOADER_H_
#define LEVEL_LOADER_H_

#include <filesystem>
#include <random>
#include <vector>

namespace sokoban {

using SokobanLevel = std::vector<uint8_t>;

constexpr uint8_t WALL = 0;
constexpr uint8_t EMPTY = 1;
constexpr uint8_t TARGET = 2;
constexpr uint8_t BOX_ON_TARGET = 3;
constexpr uint8_t BOX = 4;
constexpr uint8_t PLAYER = 5;
constexpr uint8_t PLAYER_ON_TARGET = 6;

class LevelLoader {
 protected:
  bool load_sequentially;
  int n_levels_to_load;
  int levels_loaded;
  std::vector<SokobanLevel> levels;
  std::vector<SokobanLevel>::iterator cur_level;
  std::vector<std::filesystem::path> level_file_paths;
  std::vector<std::filesystem::path>::iterator cur_file;
  void LoadFile(std::mt19937& gen);

 public:
  int verbose;

  const std::vector<SokobanLevel>::iterator GetLevel(std::mt19937& gen);
  LevelLoader(const std::filesystem::path& base_path, bool load_sequentially,
              int n_levels_to_load, int verbose = 0);
};

void PrintLevel(std::ostream& os, SokobanLevel vec);
}  // namespace sokoban

#endif  // LEVEL_LOADER_H_
