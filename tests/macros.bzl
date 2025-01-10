"""
"""

load("//:constants.bzl", "COPTS", "DEBUG_COPTS", "DYNAMIC_COPTS", "STATIC_COPTS")

COPTS_TEST = COPTS + [
    "-I.",
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
    "//src:utils.h",
    "//src:debug.h",
    "//tests/helper:headers-helper",
]

def get_copts(copts):
    """_summary_

    Args:
        copts (_type_): _description_
    """
    return (copts if copts else []) + COPTS_TEST

def get_copts_mode(mode):
    """_summary_

    Args:
        mode (_type_): _description_

    Returns:
        _type_: _description_
    """
    if mode == "dynamic":
        return DYNAMIC_COPTS
    elif mode == "static":
        return STATIC_COPTS
    else:
        return []

def get_dbg_copts(dbg):
    """_summary_

    Args:
        dbg (_type_): _description_
    """
    return DEBUG_COPTS if dbg else []

def get_deps(deps, mode, dbg):
    """
    Get the dependencies for the test.

    Args:
        deps (list): Additional dependencies.
        mode (str): The mode of the build, either "dynamic" or "static".
        dbg (bool): Whether to include debug dependencies.

    Returns:
        list: The complete list of dependencies.
    """
    _deps = DEPS + (deps if deps else [])
    if mode == "dynamic":
        prism_deps = ["@//src:prism-dynamic" + ("-dbg" if dbg else "")]
    elif mode == "static":
        prism_deps = ["@//src:prism-static" + ("-dbg" if dbg else "")]
    else:
        prism_deps = []

    return _deps + prism_deps

def cc_test_gen_scalar(name, src = None, deps = None, copts = COPTS, size = "small", dbg = False, mode = None):
    srcs = (src if src else []) + [name + ".cpp"]
    native.cc_test(
        name = name,
        srcs = srcs + HEADERS + SRCS_SCALAR,
        copts = get_copts(copts) + get_dbg_copts(dbg) + get_copts_mode(mode),
        deps = get_deps(deps, mode, dbg),
        size = size,
    )

def cc_test_gen_vector(name, src = None, deps = None, copts = COPTS, size = "small", dbg = False, mode = None):
    srcs = (src if src else []) + [name + ".cpp"]
    native.cc_test(
        name = name,
        srcs = srcs + HEADERS + SRCS_VECTOR,
        copts = get_copts(copts) + get_dbg_copts(dbg) + get_copts_mode(mode),
        deps = get_deps(deps, mode, dbg),
        size = size,
        features = ["vector"],
    )

def cc_test_lib_gen(name, src = None, deps = None, copts = COPTS, linkopts = None, size = "small", dbg = False, mode = None):
    srcs = src if src else [name + ".cpp"]
    srcs += HEADERS
    native.cc_test(
        name = name,
        srcs = srcs + SRCS,
        copts = get_copts(copts) + get_dbg_copts(dbg) + get_copts_mode(mode),
        linkopts = linkopts,
        deps = get_deps(deps, mode, dbg),
        size = size,
        visibility = ["//visibility:public"],
    )

def cc_lib_gen(name, src = None, deps = None, copts = COPTS, linkopts = None, size = "small", dbg = False, mode = None):
    srcs = src if src else [name + ".cpp"]
    srcs += HEADERS
    native.cc_library(
        name = name,
        srcs = srcs + SRCS,
        copts = get_copts(copts) + get_dbg_copts(dbg) + get_copts_mode(mode),
        linkopts = linkopts,
        deps = get_deps(deps, mode, dbg),
        size = size,
        visibility = ["//visibility:public"],
    )

def cc_test_binary(name, src = None, deps = None, copts = COPTS, linkopts = None, dbg = False, mode = None):
    srcs = src if src else [name + ".cpp"]
    srcs += HEADERS
    native.cc_binary(
        name = name,
        srcs = srcs + SRCS,
        copts = get_copts(copts) + get_dbg_copts(dbg) + get_copts_mode(mode),
        linkopts = linkopts,
        deps = get_deps(deps, mode, dbg),
        visibility = ["//visibility:public"],
    )
