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

#include <gtest/gtest.h>

#include <fstream>
#include <string>

#include "envpool/sokoban/sokoban_node.h"

namespace sokoban {

// Declare the RunAStar function from astar_log.cc
void RunAStar(const std::string& level_file_name,
              const std::string& log_file_name, int total_levels_to_run = 1000,
              int fsa_limit = 1000000);

TEST(AStarLogTest, ValidateSolution) {
  // Create a temporary file for the log
  std::string level_file_name = "/app/envpool/sokoban/sample_levels/small.txt";
  std::string log_file_name = testing::TempDir() + "/test_log_file.csv";

  // Run A* on the first level only
  RunAStar(level_file_name, log_file_name, 1);

  // Read the log file and check for the expected solution
  std::ifstream log_file(log_file_name);
  std::string line;

  // Skip header
  ASSERT_TRUE(std::getline(log_file, line));
  // Read the solution line
  ASSERT_TRUE(std::getline(log_file, line));

  const std::string expected = "0,222200001112330322210,21,1380";
  EXPECT_EQ(line, expected);
}

}  // namespace sokoban
