load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("//:constants.bzl", "COPTS", "DEBUG_COPTS", "DYNAMIC_COPTS", "RANDOM_FULLBITS_COPTS", "RANDOM_PARTIALBITS_COPTS", "STATIC_COPTS")

exports_files([
    "prism_api.cpp",
    "prism_api.h",
    "prism_array.h",
    "prism_config.cpp",
    "debug.h",
    "debug_vector-inl.h",
    "eft.h",
    "generic_vector.h",
    "generic_vector-inl.h",
    "random-inl.h",
    "seed.h",
    "sr_array.h",
    "sr_scalar.h",
    "sr_scalar_static.cpp",
    "sr_scalar_dynamic.cpp",
    "sr_vector.h",
    "sr_vector-inl.h",
    "sr_vector_static.cpp",
    "sr_vector_dynamic.cpp",
    "target_utils.h",
    "ud_array.h",
    "ud_scalar.h",
    "ud_scalar_static.cpp",
    "ud_scalar_dynamic.cpp",
    "ud_vector.h",
    "ud_vector-inl.h",
    "ud_vector_static.cpp",
    "ud_vector_dynamic.cpp",
    "utils.h",
    "xoshiro.h",
    "xoshiro_vector.cpp",
])

# File groups

# Xoshiro random number generator

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

# PRISM API headers

filegroup(
    name = "headers-prism",
    srcs = [
        "prism_api.h",
        "prism_array.h",
        "sr_array.h",
        "sr_scalar.h",
        "sr_vector.h",
        "ud_array.h",
        "ud_scalar.h",
        "ud_vector.h",
    ],
)

# PRISM dynamic implementation

filegroup(
    name = "srcs-prism-dynamic",
    srcs = glob(
        [
            "*.cpp",
            "*.h",
        ],
        exclude = [
            "*static*",
            "prism_config.cpp",
            "target_info*",
        ],
    ),
    visibility = ["//visibility:public"],
)

# PRISM static implementation

filegroup(
    name = "srcs-prism-static",
    srcs = glob(
        [
            "*.cpp",
            "*.h",
        ],
        exclude = [
            "*dynamic*",
            "prism_config.cpp",
            "target_info*",
        ],
    ),
    visibility = ["//visibility:public"],
)

# Libraries

# Process-wide state (configuration, seed, thread numbering). Every other PRISM
# library depends on it, so that they all share a single copy of the state.

cc_library(
    name = "prism-config",
    srcs = ["prism_config.cpp"],
    hdrs = [
        "debug.h",
        "seed.h",
        "utils.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
)

# Xoshiro random number generator

cc_library(
    name = "prism-rng",
    srcs = [":srcs-xoshiro"],
    hdrs = ["xoshiro.h"],
    copts = COPTS + DYNAMIC_COPTS + DEBUG_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

# PRISM dynamic dispatch library

cc_library(
    name = "prism-dynamic",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

cc_library(
    name = "prism-dynamic-dbg",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + DEBUG_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

# PRISM dynamic library using full bits for random numbers.

cc_library(
    name = "prism-dynamic-fullbits",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + RANDOM_FULLBITS_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

cc_library(
    name = "prism-dynamic-fullbits-dbg",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + DEBUG_COPTS + RANDOM_FULLBITS_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

cc_library(
    name = "prism-dynamic-partialbits",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + RANDOM_PARTIALBITS_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

cc_library(
    name = "prism-dynamic-partialbits-dbg",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + DEBUG_COPTS + RANDOM_PARTIALBITS_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

# PRISM static dispatch library

cc_library(
    name = "prism-static",
    srcs = [":srcs-prism-static"],
    hdrs = [":headers-prism"],
    copts = COPTS + STATIC_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

cc_library(
    name = "prism-static-dbg",
    srcs = [":srcs-prism-static"],
    hdrs = [":headers-prism"],
    copts = COPTS + STATIC_COPTS + DEBUG_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

# PRISM static library using full bits for random numbers.

cc_library(
    name = "prism-static-partialbits",
    srcs = [":srcs-prism-static"],
    hdrs = [":headers-prism"],
    copts = COPTS + STATIC_COPTS + RANDOM_PARTIALBITS_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

cc_library(
    name = "prism-static-partialbits-dbg",
    srcs = [":srcs-prism-static"],
    hdrs = [":headers-prism"],
    copts = COPTS + STATIC_COPTS + DEBUG_COPTS + RANDOM_PARTIALBITS_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prism-config",
        "@hwy",
    ],
)

filegroup(
    name = "prism-all",
    srcs = [
        ":prism-config",
        ":prism-dynamic",
        ":prism-dynamic-dbg",
        ":prism-static",
        ":prism-static-dbg",
    ],
)

# Reports the Highway targets of the static and dynamic libraries
# (`make target-info`): useful where -march=native may not mean what it says.

cc_library(
    name = "target-info-static",
    srcs = ["target_info_static.cpp"],
    hdrs = ["target_info.h"],
    copts = COPTS + STATIC_COPTS,
    deps = ["@hwy"],
)

cc_library(
    name = "target-info-dynamic",
    srcs = ["target_info_dynamic.cpp"],
    hdrs = ["target_info.h"],
    copts = COPTS + DYNAMIC_COPTS,
    deps = ["@hwy"],
)

cc_binary(
    name = "prism-target-info",
    srcs = ["target_info_main.cpp"],
    copts = COPTS,
    deps = [
        ":target-info-dynamic",
        ":target-info-static",
    ],
)
