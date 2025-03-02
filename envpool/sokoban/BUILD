load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")
load("@pip_requirements//:requirements.bzl", "requirement")

package(default_visibility = ["//visibility:public"])

# 1. Level Loader
cc_library(
    name = "level_loader_lib",
    srcs = ["level_loader.cc"],
    hdrs = [
        "level_loader.h",
        "utils.h",
    ],
)

# 2. Sokoban Node
cc_library(
    name = "sokoban_node_lib",
    srcs = ["sokoban_node.cc"],
    hdrs = ["sokoban_node.h"],
    deps = [
        ":level_loader_lib",
        "//third_party/astar_stl:astar_stl_h",
    ],
)

# 4. Sokoban Env
cc_library(
    name = "sokoban_envpool_lib",
    srcs = [
        "sokoban_envpool.cc",
    ],
    hdrs = [
        "sokoban_envpool.h",
        "utils.h",
    ],
    deps = [
        ":level_loader_lib",
        "//envpool/core:async_envpool",
        "//envpool/core:env",
        "//envpool/core:env_spec",
        "//envpool/core:py_envpool",
    ],
)

# 5. astar_log
cc_library(
    name = "astar_log_lib",
    srcs = [
        "astar_log.cc",
    ],
    deps = [
        ":level_loader_lib",
        ":sokoban_node_lib",
        "//third_party/astar_stl:astar_stl_h",
    ],
)

# The actual CLI tool with the main:
cc_binary(
    name = "astar_log",
    srcs = [
        "astar_log_main.cc",
    ],
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

# 6. Python tests
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

# Now your test doesn't accidentally invoke that main.
cc_test(
    name = "astar_log_test",
    srcs = [
        "astar_log_test.cc",
    ],
    deps = [
        ":astar_log_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

# 7. Python code and extension
py_library(
    name = "sokoban",
    srcs = ["__init__.py"],
    data = [":sokoban_envpool.so"],
    deps = [
        "//envpool/python:api",
    ],
)

py_library(
    name = "registration",
    srcs = ["registration.py"],
    deps = [
        "//envpool:registration",
    ],
)

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
