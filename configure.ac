m4_define([PRISM_VERSION], [m4_esyscmd([grep -oPZ '(?<=__version__ = ")[^"]*' VERSION | tr -d '\n'])])

AC_PREREQ([2.69])
AC_INIT([prism], [PRISM_VERSION], [])
LT_INIT
AM_SILENT_RULES([yes])
AC_CONFIG_AUX_DIR(autoconf)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AX_LLVM([4.0],[16.0.6],[all])
AC_SUBST(LLVM_CPPFLAGS, $LLVM_CPPFLAGS)
AC_SUBST(LLVM_LDFLAGS, $LLVM_LDFLAGS)
AC_SUBST(LLVM_VERSION_MAJOR, $LLVM_VERSION_MAJOR)

AC_PATH_PROG([CLANG], [clang], "", $LLVM_BINDIR/ $PATH)
if test -z "$CLANG"; then
   AC_PATH_PROGS([CLANG], [clang-"$LLVM_VERSION_MAJOR"."$LLVM_VERSION_MINOR" clang], "")
fi
if test -z "$CLANG"; then
  AC_MSG_ERROR([Could not find clang])
fi
AC_DEFINE_UNQUOTED([CLANG_PATH], ["$CLANG"], [CLANG path])
AC_SUBST(CLANG_PATH, $CLANG)

AC_PATH_PROG([CLANGXX], [clang++], "", $LLVM_BINDIR/ $PATH)
if test -z "$CLANGXX"; then
   AC_PATH_PROGS([CLANGXX], [clang++-"$LLVM_VERSION_MAJOR"."$LLVM_VERSION_MINOR" clang++], "")
fi
if test -z "$CLANGXX"; then
  AC_MSG_ERROR([Could not find clang++])
fi
AC_DEFINE_UNQUOTED([CLANGXX_PATH], ["$CLANGXX"], [CLANGXX path])
AC_SUBST(CLANGXX_PATH, $CLANGXX)

# Check for parallel (required for the test infrastructure)
AC_PATH_PROG(PARALLEL, [parallel], [])
if test -z "$PARALLEL"; then
  AC_MSG_ERROR([Could not find parallel])
fi

# Check for bazel
AC_PATH_PROG(BAZEL, [bazel], [])
if test -z "$BAZEL"; then
  AC_PATH_PROG(BAZEL, [bazelisk], [])
fi
if test -z "$BAZEL"; then
  AC_MSG_ERROR([Could not find bazel])
fi

# Checks if clang support -march=native
AC_MSG_CHECKING([if $CLANG supports -march=native])
doit='$CLANG -march=native -E - </dev/null >/dev/null 2>&1'
if AC_TRY_EVAL(doit); then
  AC_DEFINE_UNQUOTED([MARCH_FLAG], ["-march=native"], [The -march=native flag])
  AC_SUBST(MARCH_FLAG, "-march=native")
  AC_MSG_RESULT([yes])
else
  AC_DEFINE_UNQUOTED([MARCH_FLAG], ["-mtune=native"], [The -mtune=native flag])
  AC_SUBST(MARCH_FLAG, "-mtune=native")
  AC_MSG_RESULT([no])
fi

# Generate config files 
AC_CONFIG_FILES([.bazelrc install.sh],
                [chmod +x install.sh])

AC_OUTPUT
