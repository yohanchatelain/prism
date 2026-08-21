load("//:constants.bzl", "DYNAMIC_COPTS", "STATIC_COPTS")
load("//tests:macros.bzl", "cc_test_gen_array", "cc_test_lib_gen")

# Boundary and variable size tests (dynamic & static)

cc_test_gen_array(
    name = "test_array_boundary",
    mode = "dynamic",
)

cc_test_gen_array(
    name = "test_array_boundary_static",
    src = [":test_array_boundary.cpp"],
    mode = "static",
)

# In-place pointer aliasing tests

cc_test_gen_array(
    name = "test_array_inplace",
    mode = "dynamic",
)

cc_test_gen_array(
    name = "test_array_inplace_static",
    src = [":test_array_inplace.cpp"],
    mode = "static",
)

# Special IEEE-754 values tests (NaN, Inf, 0, subnormals)

cc_test_gen_array(
    name = "test_array_special_values",
    mode = "dynamic",
)

# Configuration tests (virtual precision, RN mode)

cc_test_gen_array(
    name = "test_array_config",
    mode = "dynamic",
)

# C-API verification tests

cc_test_lib_gen(
    name = "test_array_c_api",
    src = [
        ":test_array_c_api.cpp",
        "//src:prism_api.h",
    ],
    mode = "dynamic",
)

# Accuracy & statistical distribution tests for Array Interface

cc_test_lib_gen(
    name = "test_array_accuracy",
    size = "medium",
    src = [":test_array_accuracy.cpp"],
    mode = "dynamic",
)

# Test suite containing all array tests

test_suite(
    name = "all",
    tests = [
        ":test_array_boundary",
        ":test_array_boundary_static",
        ":test_array_c_api",
        ":test_array_config",
        ":test_array_inplace",
        ":test_array_inplace_static",
        ":test_array_special_values",
        ":test_array_accuracy",
    ],
)
