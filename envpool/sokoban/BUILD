load("@pip_requirements//:requirements.bzl", "requirement")
load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

package(default_visibility = ["//visibility:public"])

# 1. Level Loader
cc_library(
    name = "level_loader_lib",
    hdrs = ["level_loader.h", "utils.h"],
    srcs = ["level_loader.cc"],
    # if needed, add any includes or linkopts here
)

# 2. Sokoban Node
cc_library(
    name = "sokoban_node_lib",
    hdrs = ["sokoban_node.h"],        # "level_loader.h" is #included from .h if needed
    srcs = ["sokoban_node.cc"],
    deps = [
        "//third_party/astar_stl:astar_stl_h",
        ":level_loader_lib",         # only if SokobanNode uses LevelLoader calls
        # If "utils.h" is used in sokoban_node.cc, you can add a tiny library for it
        # or just treat it as header-only. If header-only, you may not need an extra dep.
    ],
)

# 3. (Optional) If utils.h is purely header-only, you can omit a library. Otherwise:
# cc_library(
#     name = "sokoban_utils_lib",
#     hdrs = ["utils.h"],
#     # no srcs if purely templates header-only, or "utils.cc" if it exists
# )

# 4. Sokoban Env
cc_library(
    name = "sokoban_envpool_lib",
    hdrs = [
        "sokoban_envpool.h",
    ],
    srcs = [
        "sokoban_envpool.cc",
    ],
    deps = [
        # these come from your existing config
        "//envpool/core:async_envpool",
        "//envpool/core:env",
        "//envpool/core:env_spec",
        ":level_loader_lib",  # needed because we actually instantiate LevelLoader in .cc
        # If "utils.h" needed a separate library, you'd add it here too
    ],
)

# 5. astar_log
cc_library(
    name = "astar_log_lib",
    srcs = [
        "astar_log.cc",  # no main here
    ],
    deps = [
        ":sokoban_node_lib",
        ":level_loader_lib",
        "//third_party/astar_stl:astar_stl_h",
        # anything else as needed
    ],
)

# The actual CLI tool with the main:
cc_binary(
    name = "astar_log",
    srcs = [
        "astar_log_main.cc",  # <--- main() is here now
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
        ":sokoban",   # lbry below
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
        "//envpool/core:py_envpool",  # presumably needed for the PyEnvSpec/PyEnvPool macros
    ],
)