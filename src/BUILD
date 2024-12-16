exports_files([
    "debug.h",
    "debug_vector-inl.h",
    "eft.h",
    "random-inl.h",
    "sr_vector.h",
    "sr_scalar.h",
    "sr_vector-inl.h",
    "sr_vector.cpp",
    "target_utils.h",
    "ud_scalar.h",
    "ud_vector.h",
    "ud_vector.cpp",
    "ud_vector-inl.h",
    "utils.h",
    "xoshiro.h",
    "xoshiro_vector.cpp",
])

# Compiler options
COPTS = [
    "-std=c++17",
    "-Wfatal-errors",
    "-O2",
    "-Wall",
    "-Wno-psabi",
]

filegroup(
    name = "srcs-xoshiro",
    srcs = [
        "debug_vector-inl.h",
        "random-inl.h",
        "target_utils.h",
        "xoshiro.h",
        "xoshiro_vector.cpp",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "srcs-prism",
    srcs = glob(
        [
            "*.cpp",
            "*.h",
        ],
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "headers-prism",
    srcs =
        [
            "//src:sr_vector.h",
            "//src:ud_vector.h",
        ],
)

cc_library(
    name = "prism",
    srcs = [":srcs-prism"],
    hdrs = [":headers-prism"],
    copts = COPTS + [
        "-DHWY_COMPILE_ALL_ATTAINABLE",
    ],
    visibility = ["//visibility:public"],
    deps = ["@hwy"],
)

cc_library(
    name = "prism-dbg",
    srcs = [":srcs-prism"],
    copts = COPTS + ["-DPRISM_DEBUG"],
    visibility = ["//visibility:public"],
    deps = [
        ":prism",
        "@hwy",
    ],
)

cc_library(
    name = "prism-native",
    srcs = [":srcs-prism"],
    hdrs = [":headers-prism"],
    copts = COPTS + [
        "-march=native",
        "-DWARN_FMA_EMULATION",
        "-DHWY_COMPILE_ONLY_STATIC",
    ],
    visibility = ["//visibility:public"],
    deps = ["@hwy"],
)

cc_library(
    name = "prism-native-dbg",
    srcs = [":srcs-prism"],
    copts = COPTS + [
        "-DPRISM_DEBUG",
        "-DWARN_FMA_EMULATION",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":prism-native",
        "@hwy",
    ],
)

filegroup(
    name = "prism-all",
    srcs = [
        ":prism",
        ":prism-dbg",
        ":prism-native",
        ":prism-native-dbg",
    ],
)
