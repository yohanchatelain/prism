"""
"""

# Compiler options
COPTS = [
    "-std=c++17",
    "-Wfatal-errors",
    "-O2",
    "-Wall",
    "-Wno-psabi",
]

NATIVE_COPTS = [
    "-march=native",
    "-mtune=native",
]

STATIC_COPTS = [
    "-DWARN_FMA_EMULATION",
    "-DHWY_COMPILE_ONLY_STATIC",
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
