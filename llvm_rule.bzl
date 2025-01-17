load("//:constants.bzl", "COPTS", "DYNAMIC_COPTS", "STATIC_COPTS")

IR_COPTS = [
    "-S",
    "-emit-llvm",
    "--save-temps",
]

def get_copts(dispatch):
    """Returns the compiler options for the given dispatch mode.

    Args:
        dispatch: The dispatch mode, either "dynamic" or "static".

    Returns:
        A list of compiler options.
    """
    if dispatch == "dynamic":
        return COPTS + DYNAMIC_COPTS
    elif dispatch == "static":
        return COPTS + STATIC_COPTS
    else:
        fail("Invalid dispatch mode: " + dispatch)

def get_deps(dispatch_mode):
    """Returns the dependencies for the given dispatch mode.

    Args:
        dispatch_mode: The dispatch mode, either "dynamic" or "static".

    Returns:
        A list of dependencies.
    """
    if dispatch_mode == "dynamic":
        return ["//src:prism-" + dispatch_mode]
    elif dispatch_mode == "static":
        return ["//src:prism-" + dispatch_mode]
    else:
        fail("Invalid dispatch mode: " + dispatch_mode)

def get_name(prmode, dispatch, type):
    return prmode + "_" + type + "_" + dispatch + "_ll"

def get_srcs(prmode, dispatch, type):
    return ["//src:" + prmode + "_" + type + "_" + dispatch + ".cpp"]

def llvm_ir_gen(name, prmode, dispatch, type):
    native.cc_library(
        name = name,
        srcs = get_srcs(prmode, dispatch, type),
        copts = IR_COPTS + get_copts(dispatch),
        deps = get_deps(dispatch),
        visibility = ["//visibility:public"],
        linkstatic = True,
    )
