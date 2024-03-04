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

cc_library(
    name = "sokoban_envpool_h",
    hdrs = [
        "level_loader.h",
        "sokoban_envpool.h",
    ],
    deps = [
        "//envpool/core:async_envpool",
        "//envpool/core:env",
        "//envpool/core:env_spec",
        "//envpool/utils:safe_random_h",
    ],
)

# cc_test(
#    name = "sokoban_envpool_test",
#    size = "enormous",
#    srcs = ["sokoban_envpool_test.cc"],
#    deps = [
#        ":sokoban_envpool_h",
#        "@com_google_googletest//:gtest_main",
#    ],
# )

py_test(
    name = "test",
    srcs = ["sokoban_py_envpool_test.py"],
    main = "sokoban_py_envpool_test.py",
    deps = [
        ":registration",
        ":sokoban",
        "//envpool",
        requirement("numpy"),
        requirement("absl-py"),
    ],
)

pybind_extension(
    name = "sokoban_envpool",
    srcs = [
        "level_loader.cc",
        "sokoban_envpool.cc",
    ],
    linkopts = [
        "-ldl",
    ],
    deps = [
        ":sokoban_envpool_h",
        "//envpool/core:py_envpool",
    ],
)
