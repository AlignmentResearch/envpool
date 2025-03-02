load("@envpool//third_party:common.bzl", "copy_directory")

copy_directory(
    name = "roms",
    src = "roms_sources",
    out = "roms",
    visibility = ["//visibility:public"],
)


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
    )
)
