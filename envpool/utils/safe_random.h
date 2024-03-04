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

#ifndef ENVPOOL_UTILS_SAFE_RANDOM_H_
#define ENVPOOL_UTILS_SAFE_RANDOM_H_

#include <glog/logging.h>

#include <random>
#include <type_traits>

namespace envpool {

template <typename IntType>
std::uniform_int_distribution<IntType> SafeUniformIntDistribution(IntType a,
                                                                  IntType b) {
  // Prevent UB from a > b.
  // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution/operator()
  CHECK_LE(a, b);
  // Prevent UB from IntType not being one of the blessed ones.
  // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
  static_assert(
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, short> ||
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, int> ||
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, long> ||
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, long long> ||
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, unsigned short> ||
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, unsigned int> ||
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, unsigned long> ||
      // NOLINTNEXTLINE(google-runtime-int)
      std::is_same_v<IntType, unsigned long long>);

  return std::uniform_int_distribution<IntType>(a, b);
}
}  // namespace envpool

#endif  // ENVPOOL_UTILS_SAFE_RANDOM_H_
