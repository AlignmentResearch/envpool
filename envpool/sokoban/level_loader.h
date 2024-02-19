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

enum LevelChoiceMode {CHOICE_RANDOM=0, CHOICE_SEQUENTIAL=1};

class LevelLoader {
 protected:
  std::vector<SokobanLevel> levels_;
  std::vector<SokobanLevel>::iterator cur_level_;
  const std::vector<std::filesystem::path> level_file_paths_;
  std::vector<std::filesystem::path>::const_iterator level_file_paths_iterator_;
  std::filesystem::path ChooseRandomFile(std::mt19937& gen);

  /* Loads the levels from `file_path` and stores them in `levels` */
  void LoadNewFile(const std::filesystem::path &file_path);

 public:
  int verbose_;
  LevelChoiceMode choice_mode_;

  const std::vector<SokobanLevel>::iterator RandomLevel(std::mt19937& gen);
  LevelLoader(const std::filesystem::path& base_path, LevelChoiceMode choice_mode, int verbose=0);
};


void PrintLevel(std::ostream& os, SokobanLevel vec);
}  // namespace sokoban

#endif  // LEVEL_LOADER_H_
