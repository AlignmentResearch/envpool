load("@pip_requirements//:requirements.bzl", "requirement")
load("@rules_python//python:packaging.bzl", "py_package", "py_wheel", "py_wheel_dist")


filegroup(
    name = "clang_tidy_config",
    data = [".clang-tidy"],
)

py_binary(
    name = "setup",
    srcs = [
        "setup.py",
    ],
    data = [
        "README.md",
        "setup.cfg",
        "//envpool",
    ],
    main = "setup.py",
    python_version = "PY3",
    deps = [
        requirement("setuptools"),
        requirement("wheel"),
    ],
)

# Collect transitive dependencies of envpool
py_package(
    name = "pkg",
    packages = [],
    deps = ["//envpool:envpool"],
)

py_wheel(
    name = "wheel",
    distribution = "envpool",
    python_tag = "cp312",
    abi = "cp312",
    platform="linux_x86_64",
    twine = None,
    version = "0.9.0",
    deps = [
        ":pkg",
    ],
)
py_wheel_dist(name="wheel_dist", out="dist", wheel="wheel")
