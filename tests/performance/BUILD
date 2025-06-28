load("//:constants.bzl", "DYNAMIC_COPTS")
load("//tests:macros.bzl", "cc_test_lib_gen")

cc_test_lib_gen(
    name = "performance-regression",
    src = [
        "performance_regression_test.cpp",
        "benchmark_suite.h",
    ],
    copts = DYNAMIC_COPTS + [
        "-O3",
        "-march=native",
        "-DNDEBUG",
    ],
    mode = "dynamic",
    size = "large",
)

filegroup(
    name = "headers-performance",
    srcs = [
        "benchmark_suite.h",
    ],
    visibility = ["//visibility:public"],
)