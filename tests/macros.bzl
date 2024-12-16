"""
"""

COPTS = [
    "-std=c++17",
    "-I.",
    "-Wfatal-errors",
    "-DHWY_IS_TEST",
]

SRCS = []

SRCS_SCALAR = [
    "//src:eft.h",
]

SRCS_VECTOR = [
    "//src:sr_vector-inl.h",
    "//src:ud_vector-inl.h",
    "//src:debug_vector-inl.h",
    "//src:xoshiro.h",
    "//src:random-inl.h",
    "//src:target_utils.h",
]

DEPS = [
    "@hwy",
    "@hwy//:hwy_test_util",
    "@googletest//:gtest",
    "@googletest//:gtest_main",
    "@boost//:math",
    "@boost//:multiprecision",
]

HEADERS = [
    "//tests:helper.h",
    "//src:utils.h",
    "//src:debug.h",
]

def cc_test_gen_scalar(name, src = None, deps = DEPS, copts = COPTS, size = "small", dbg = False):
    srcs = (src if src else []) + [name + ".cpp"]
    native.cc_test(
        name = name,
        srcs = srcs + HEADERS + SRCS_SCALAR,
        copts = COPTS + (copts if copts else []) + (["-DPRISM_DEBUG"] if dbg else []),
        deps = deps,
        size = size,
    )

def cc_test_gen_vector(name, src = None, deps = DEPS, copts = COPTS, size = "small", dbg = False):
    srcs = (src if src else []) + [name + ".cpp"]
    native.cc_test(
        name = name,
        srcs = srcs + HEADERS + SRCS_VECTOR,
        copts = COPTS + (copts if copts else []) + (["-DPRISM_DEBUG"] if dbg else []),
        deps = deps,
        size = size,
        features = ["vector"],
    )

def cc_test_lib_gen(name, src = None, deps = None, copts = COPTS, size = "small", dbg = False):
    srcs = src if src else [name + ".cpp"]
    srcs += HEADERS
    native.cc_test(
        name = name,
        srcs = srcs + SRCS,
        copts = COPTS + (copts if copts else []) + (["-DPRISM_DEBUG"] if dbg else []),
        deps = DEPS + (deps if deps else []),
        size = size,
        visibility = ["//visibility:public"],
    )
