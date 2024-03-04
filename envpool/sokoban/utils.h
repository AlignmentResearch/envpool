#ifndef SOKOBAN_UTILS_H
#define SOKOBAN_UTILS_H

#include <random>

namespace sokoban {

template <typename T>
T safe_uniform_int(T low, T high, std::mt19937& gen) {
  // check if low is greater than high
  if (low > high) {
    throw std::invalid_argument("low should be less than high");
  }
  static_assert(
      std::is_same_v<T, unsigned> || std::is_same_v<T, unsigned long> ||
          std::is_same_v<T, unsigned long long> || std::is_same_v<T, int> ||
          std::is_same_v<T, long> || std::is_same_v<T, long long>,
      "safe_uniform_int only supports int, long, and long long");
  std::uniform_int_distribution<T> dist(low, high);
  return dist(gen);
}

}  // namespace sokoban

#endif  // SOKOBAN_UTILS_H