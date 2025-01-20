load("//:config.bzl", "LINK")
load("//:llvm_rule.bzl", "llvm_ir_gen")

# Build the LLVM IR for the scalar dynamic dispatch mode

# Up-down mode

# Scalar dynamic dispatch mode

llvm_ir_gen(
    name = "_ud_scalar_dynamic_ll",
    dispatch = "dynamic",
    prmode = "ud",
    type = "scalar",
)

genrule(
    name = "ud_scalar_dynamic_ll",
    srcs = [":_ud_scalar_dynamic_ll"],
    outs = ["ud_scalar_dynamic.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# Scalar static dispatch mode

llvm_ir_gen(
    name = "_ud_scalar_static_ll",
    dispatch = "static",
    prmode = "ud",
    type = "scalar",
)

genrule(
    name = "ud_scalar_static_ll",
    srcs = [":_ud_scalar_static_ll"],
    outs = ["ud_scalar_static.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# Vector dynamic dispatch mode

llvm_ir_gen(
    name = "_ud_vector_dynamic_ll",
    dispatch = "dynamic",
    prmode = "ud",
    type = "vector",
)

genrule(
    name = "ud_vector_dynamic_ll",
    srcs = [":_ud_vector_dynamic_ll"],
    outs = ["ud_vector_dynamic.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# Vector static dispatch mode

llvm_ir_gen(
    name = "_ud_vector_static_ll",
    dispatch = "static",
    prmode = "ud",
    type = "vector",
)

genrule(
    name = "ud_vector_static_ll",
    srcs = [":_ud_vector_static_ll"],
    outs = ["ud_vector_static.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# SR mode

# Scalar dynamic dispatch mode

llvm_ir_gen(
    name = "_sr_scalar_dynamic_ll",
    dispatch = "dynamic",
    prmode = "sr",
    type = "scalar",
)

genrule(
    name = "sr_scalar_dynamic_ll",
    srcs = [":_sr_scalar_dynamic_ll"],
    outs = ["sr_scalar_dynamic.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# Scalar static dispatch mode

llvm_ir_gen(
    name = "_sr_scalar_static_ll",
    dispatch = "static",
    prmode = "sr",
    type = "scalar",
)

genrule(
    name = "sr_scalar_static_ll",
    srcs = [":_sr_scalar_static_ll"],
    outs = ["sr_scalar_static.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# Vector dynamic dispatch mode

llvm_ir_gen(
    name = "_sr_vector_dynamic_ll",
    dispatch = "dynamic",
    prmode = "sr",
    type = "vector",
)

genrule(
    name = "sr_vector_dynamic_ll",
    srcs = [":_sr_vector_dynamic_ll"],
    outs = ["sr_vector_dynamic.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# Vector static dispatch mode

llvm_ir_gen(
    name = "_sr_vector_static_ll",
    dispatch = "static",
    prmode = "sr",
    type = "vector",
)

genrule(
    name = "sr_vector_static_ll",
    srcs = [":_sr_vector_static_ll"],
    outs = ["sr_vector_static.ll"],
    cmd = "sed -n '/; ModuleID =.*/,$$p' $< > $@",
)

# Build prism-dynamic.ll

genrule(
    name = "prism_dynamic_ll",
    srcs = [
        ":ud_scalar_dynamic_ll",
        ":ud_vector_dynamic_ll",
        ":sr_scalar_dynamic_ll",
        ":sr_vector_dynamic_ll",
    ],
    outs = ["prism-dynamic.ll"],
    cmd = LINK + " $(SRCS) > $@",
)

# Build prism-static.ll

genrule(
    name = "prism_static_ll",
    srcs = [
        ":ud_scalar_static_ll",
        ":ud_vector_static_ll",
        ":sr_scalar_static_ll",
        ":sr_vector_static_ll",
    ],
    outs = ["prism-static.ll"],
    cmd = LINK + " $(SRCS) > $@",
)

filegroup(
    name = "prism_ir",
    srcs = [
        ":prism_dynamic_ll",
        ":prism_static_ll",
    ],
)
