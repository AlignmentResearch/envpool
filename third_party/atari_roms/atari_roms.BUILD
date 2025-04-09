load("@envpool//third_party:common.bzl", "copy_directory")

filegroup(
    name = "roms_sources",
    srcs = glob(
        ["ROM/*/*.bin"],
        exclude = [
            "ROM/combat/combat.bin",
            "ROM/joust/joust.bin",
            "ROM/maze_craze/maze_craze.bin",
            "ROM/warlords/warlords.bin",
        ],
    ),
    visibility=["//visibility:public"],
)
