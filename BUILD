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
    deps = ["//envpool"],
)

py_wheel(
    name = "wheel",
    abi = "cp312",
    distribution = "far_envpool",
    platform = "manylinux2014_x86_64",
    python_tag = "cp312",
    requires=[
        "numpy>=2.2.0",
        "dm-env>=1.6",
        "gym>=0.26",
        "gymnasium>=0.26,!=0.27.0",
        "optree>=0.6.0",
        "jax>=0.5.0",
        "pytest",
    ],
    python_requires = ">=3.10,<3.13",
    twine = None,
    version = "0.9.0",
    deps = [
        ":pkg",
    ],
)

py_wheel_dist(
    name = "wheel_dist",
    out = "dist",
    wheel = "wheel",
)
