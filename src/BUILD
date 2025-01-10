load("//:constants.bzl", "COPTS", "DEBUG_COPTS", "DYNAMIC_COPTS", "NATIVE_COPTS", "STATIC_COPTS")

exports_files([
    "debug.h",
    "debug_vector-inl.h",
    "eft.h",
    "random-inl.h",
    "sr_scalar.h",
    "sr_scalar_static.cpp",
    "sr_scalar_dynamic.cpp",
    "sr_vector.h",
    "sr_vector-inl.h",
    "sr_vector_static.cpp",
    "sr_vector_dynamic.cpp",
    "target_utils.h",
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
    name = "headers-prism",
    srcs = [
        "sr_scalar.h",
        "sr_vector.h",
        "ud_scalar.h",
        "ud_vector.h",
    ],
)

filegroup(
    name = "srcs-prism-dynamic",
    srcs = glob(
        [
            "*.cpp",
            "*.h",
        ],
        exclude = [
            "*static*",
        ],
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "srcs-prism-static",
    srcs = glob(
        [
            "*.cpp",
            "*.h",
        ],
        exclude = [
            "*dynamic*",
        ],
    ),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "prism-rng",
    srcs = [":srcs-xoshiro"],
    hdrs = ["xoshiro.h"],
    copts = COPTS + [
        "-DHWY_COMPILE_ALL_ATTAINABLE",
        "-g",
    ],
    visibility = ["//visibility:public"],
    deps = ["@hwy"],
)

cc_library(
    name = "prism-dynamic",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "@hwy",
    ],
)

cc_library(
    name = "prism-dynamic-dbg",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + DEBUG_COPTS,
    linkopts = ["-fsanitize=address"],
    visibility = ["//visibility:public"],
    deps = [
        "@hwy",
    ],
)

# Version of the dynamic library that uses full bits for random numbers.
cc_library(
    name = "prism-dynamic-fullbits",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + [
        "-DPRISM_UD_RANDOM_FULLBITS",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@hwy",
    ],
)

cc_library(
    name = "prism-dynamic-fullbits-dbg",
    srcs = [":srcs-prism-dynamic"],
    hdrs = [":headers-prism"],
    copts = COPTS + DYNAMIC_COPTS + DEBUG_COPTS + [
        "-DPRISM_UD_RANDOM_FULLBITS",
    ],
    linkopts = ["-fsanitize=address"],
    visibility = ["//visibility:public"],
    deps = [
        "@hwy",
    ],
)

cc_library(
    name = "prism-static",
    srcs = [":srcs-prism-static"],
    hdrs = [":headers-prism"],
    copts = COPTS + NATIVE_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "@hwy",
    ],
)

cc_library(
    name = "prism-static-dbg",
    srcs = [":srcs-prism-static"],
    hdrs = [":headers-prism"],
    copts = COPTS + STATIC_COPTS + DEBUG_COPTS,
    linkopts = ["-fsanitize=address"],
    visibility = ["//visibility:public"],
    deps = [
        "@hwy",
    ],
)

filegroup(
    name = "prism-all",
    srcs = [
        ":prism-dynamic",
        ":prism-dynamic-dbg",
        ":prism-static",
        ":prism-static-dbg",
    ],
)
