# Copyright 2023-2024 FAR AI
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@pip_requirements//:requirements.bzl", "requirement")
load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

package(default_visibility = ["//visibility:public"])

# Core C++ libraries
cc_library(
    name = "sokoban_envpool_lib",
    hdrs = [
        "level_loader.h",
        "sokoban_envpool.h",
        "utils.h",
    ],
    srcs = [
        "sokoban_envpool.cc",
    ],
    deps = [
        "//envpool/core:async_envpool",
        "//envpool/core:env",
        "//envpool/core:env_spec",
    ],
)

cc_library(
    name = "sokoban_node_h",
    hdrs = [
        "level_loader.h",
        "sokoban_node.h",
        "utils.h",
    ],
    deps = ["//third_party/astar_stl:astar_stl_h"],
)

cc_library(
    name = "level_loader_lib",
    hdrs = ["level_loader.h"],
    srcs = ["level_loader.cc"],
)

cc_library(
    name = "sokoban_node_lib",
    srcs = ["sokoban_node.cc"],
    deps = [
        ":sokoban_node_h",
    ],
)

cc_library(
    name = "astar_log_lib",
    srcs = ["astar_log.cc"],
    deps = [
        ":level_loader_lib",
        ":sokoban_node_lib",
    ],
)

# Binaries
cc_binary(
    name = "astar_log",
    deps = [
        ":astar_log_lib",
    ],
)

cc_binary(
    name = "astar_log_level",
    srcs = ["astar_log_level.cc"],
    deps = [
        ":level_loader_lib",
        ":sokoban_node_lib",
    ],
)

# Tests
py_test(
    name = "test",
    srcs = ["sokoban_py_envpool_test.py"],
    main = "sokoban_py_envpool_test.py",
    deps = [
        ":registration",
        ":sokoban",
        "//envpool",
        requirement("numpy"),
        requirement("pytest"),
    ],
)

cc_test(
    name = "astar_log_test",
    srcs = ["astar_log_test.cc"],
    deps = [
        ":astar_log_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

# Python code
py_library(
    name = "sokoban",
    srcs = ["__init__.py"],
    data = [":sokoban_envpool.so"],
    deps = ["//envpool/python:api"],
)

py_library(
    name = "registration",
    srcs = ["registration.py"],
    deps = [
        "//envpool:registration",
    ],
)


# Python extension
pybind_extension(
    name = "sokoban_envpool",
    linkopts = [
        "-ldl",
    ],
    deps = [
        ":sokoban_envpool_lib",
        "//envpool/core:py_envpool",
    ],
)