load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_shared_library.bzl", "cc_shared_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("//:constants.bzl", "COPTS", "DYNAMIC_COPTS", "STATIC_COPTS")

# verificarlo/prism#24: run-time options must reach libprism-static.so when an
# uninstrumented program dlopens an instrumented library.

cc_library(
    name = "backend-lib",
    srcs = ["backend.cpp"],
    copts = COPTS + DYNAMIC_COPTS,
    deps = ["//src:prism-dynamic"],
)

cc_shared_library(
    name = "backend",
    dynamic_deps = ["//lib:prism-dynamic"],
    shared_lib_name = "libtest_backend.so",
    deps = [":backend-lib"],
)

cc_library(
    name = "instrumented-lib",
    srcs = ["instrumented.cpp"],
    copts = COPTS + STATIC_COPTS,
    deps = ["//src:prism-static"],
)

cc_shared_library(
    name = "instrumented",
    dynamic_deps = ["//lib:prism-static"],
    shared_lib_name = "libtest_instrumented.so",
    deps = [":instrumented-lib"],
)

# Links no PRISM library: both are reached only through dlopen(RTLD_LOCAL).
cc_test(
    name = "test_dlopen_config",
    size = "small",
    srcs = [
        "test_dlopen_config.cpp",
        "//src:prism_api.h",
    ],
    args = [
        "./$(rootpath :backend)",
        "./$(rootpath :instrumented)",
    ],
    copts = COPTS,
    data = [
        ":backend",
        ":instrumented",
    ],
    linkopts = ["-ldl"],
)
