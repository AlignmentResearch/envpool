load("@pip_requirements//:requirements.bzl", "requirement")
load("@rules_python//python:packaging.bzl", "py_wheel")


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

py_wheel(
    name = "wheel",
    testonly = True,
    distribution = "envpool",
    python_tag = "cp312-cp312-linux_x86_64",
    twine = None,
    version = "0.9.0",
    deps = [
        "//envpool:envpool",
    ],
)
