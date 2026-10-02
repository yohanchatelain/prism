"""Installed shared libraries.

The .so files Bazel builds implicitly for cc_library targets record no
dependencies: libprism-static.so would not list libprism-config.so or
libhwy.so as NEEDED, so it could only be loaded where something else had
already loaded those libraries into the global scope (verificarlo/prism#23).
The cc_shared_library targets below link each library against the shared
libraries it uses instead, and find them next to itself through $ORIGIN.
"""

load("@rules_cc//cc:cc_shared_library.bzl", "cc_shared_library")

package(default_visibility = ["//visibility:public"])

def_link_flags = ["-Wl,-rpath,$ORIGIN"]

cc_shared_library(
    name = "hwy",
    shared_lib_name = "libhwy.so",
    user_link_flags = ["-Wl,-soname,libhwy.so"],
    deps = ["@hwy"],
)

# Holds the state every other PRISM library shares (verificarlo/prism#24).
cc_shared_library(
    name = "prism-config",
    shared_lib_name = "libprism-config.so",
    user_link_flags = ["-Wl,-soname,libprism-config.so"],
    deps = ["//src:prism-config"],
)

PRISM_LIBRARIES = [
    "prism-dynamic",
    "prism-dynamic-dbg",
    "prism-dynamic-fullbits",
    "prism-dynamic-fullbits-dbg",
    "prism-dynamic-partialbits",
    "prism-dynamic-partialbits-dbg",
    "prism-static",
    "prism-static-dbg",
    "prism-static-partialbits",
    "prism-static-partialbits-dbg",
]

[
    cc_shared_library(
        name = library,
        dynamic_deps = [
            ":hwy",
            ":prism-config",
        ],
        shared_lib_name = "lib" + library + ".so",
        user_link_flags = def_link_flags + ["-Wl,-soname,lib" + library + ".so"],
        deps = ["//src:" + library],
    )
    for library in PRISM_LIBRARIES
]

filegroup(
    name = "shared-libraries",
    srcs = [
        ":hwy",
        ":prism-config",
        ":prism-dynamic",
        ":prism-dynamic-dbg",
        ":prism-static",
        ":prism-static-dbg",
    ],
)
