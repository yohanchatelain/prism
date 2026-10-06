"""
"""

load("//:config.bzl", "EXPERIMENTAL_SVE", "MARCH_FLAG")

# Compiler options

# Slow, but full-precision random number generation.
RANDOM_FULLBITS_COPTS = [
    "-DPRISM_RANDOM_FULLBITS",
]

# Fast, but partial-precision random number generation.
RANDOM_PARTIALBITS_COPTS = [
    "-UPRISM_RANDOM_FULLBITS",
]

# Arm SVE targets are experimental and opt-in (./configure
# --enable-experimental-sve). When disabled, aarch64 builds use NEON only. This
# is a no-op on other architectures.
TARGET_COPTS = [] if EXPERIMENTAL_SVE else [
    "-DHWY_DISABLED_TARGETS=HWY_ALL_SVE",
]

COPTS = [
    "-std=c++17",
    "-Wfatal-errors",
    "-O2",
    "-Wall",
    "-Wno-psabi",
] + RANDOM_PARTIALBITS_COPTS + TARGET_COPTS

NATIVE_COPTS = [
    MARCH_FLAG,
]

STATIC_COPTS = [
    "-DWARN_FMA_EMULATION",
    "-DHWY_COMPILE_ONLY_STATIC",
    # Highway's SSE4, AVX2 and AVX3 targets also require AES and PCLMUL, which
    # the x86-64-v2/v3/v4 levels do not include. PRISM does not use either, so
    # without this, --with-arch=x86-64-v{2,3,4} silently falls back to SSSE3.
    "-DHWY_DISABLE_PCLMUL_AES",
    "-DPRISM_DISPATCH=static_dispatch",
] + NATIVE_COPTS

DYNAMIC_COPTS = [
    "-DHWY_COMPILE_ALL_ATTAINABLE",
    "-DPRISM_DISPATCH=dynamic_dispatch",
]

DEBUG_COPTS = [
    "-DPRISM_DEBUG",
    "-Og",
    "-g",
    "-fno-omit-frame-pointer",
]

SANITIZE_COPTS = [
    "-fsanitize=address",
    "-fsanitize=undefined",
    "-fsanitize=leak",
    "-fsanitize=thread",
]
