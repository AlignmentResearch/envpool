load("@pip_requirements//:requirements.bzl", "requirement")
load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

package(default_visibility=["//visibility:public"])

py_library(
    name="sokoban",
    srcs=["__init__.py"],
    data=[":sokoban_envpool.so"],
    deps=["//envpool/python:api"],
)

py_library(
    name="registration",
    srcs=["registration.py"],
    deps=[
        "//envpool:registration",
    ],
)

cc_library(
    name="sokoban_envpool_h",
    hdrs=["sokoban_envpool.h", "level_loader.h"],
    deps=[
        "//envpool/core:async_envpool",
        "//envpool/core:env",
        "//envpool/core:env_spec",
    ],
)

cc_library(
    name="sokoban_node_h",
    hdrs=["sokoban_node.h", "level_loader.h", "fsa.h", "astar.h"],
    deps=[],
)

cc_test(
    name="sokoban_astar_test",
    size="enormous",
    srcs=["sokoban_astar_test.cc", "sokoban_node.cc", "level_loader.cc"],
    deps=[
        ":sokoban_node_h",
        "@com_github_google_glog//:glog",
        "@com_google_googletest//:gtest_main",
    ],
)

py_test(
    name="test",
    main="sokoban_py_envpool_test.py",
    srcs=["sokoban_py_envpool_test.py"],
    deps=[
        ":sokoban",
        ":registration",
        "//envpool:envpool",
        requirement("numpy"),
        requirement("absl-py"),
    ],
)

pybind_extension(
    name="sokoban_envpool",
    srcs=[
        "sokoban_envpool.cc",
        "level_loader.cc",
    ],
    linkopts=[
        "-ldl",
    ],
    deps=[
        ":sokoban_envpool_h",
        "//envpool/core:py_envpool",
    ],
)
