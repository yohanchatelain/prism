
```bash
_|_|_|    _|_|_|    _|_|_|    _|_|_|  _|      _|
_|    _|  _|    _|    _|    _|        _|_|  _|_|
_|_|_|    _|_|_|      _|      _|_|    _|  _|  _|
_|        _|    _|    _|          _|  _|      _|
_|        _|    _|  _|_|_|  _|_|_|    _|      _|
```
---

# Probabilistic Rounding with Instruction Set Management

This library provides a **vectorized implementation** of two probabilistic rounding modes:

1. **Up-Down Rounding Mode**: Add +/- 1 ulp with equal probabilities (1/2). Do not preserve exact operations.
2. **Stochastic Rounding**: As described in [Fasi and Mikaitis: Algorithms for Stochastically Rounded Elementary Arithmetic Operations](https://ieeexplore.ieee.org/document/9387551), extended here to support the FMA operator.

The library leverages the [Highway library](https://github.com/google/highway), a high-performance C++ library for portable vector instructions across platforms. It uses **dynamic dispatch** to efficiently execute functions across different architectures. 

### Features

The library is available in three interfaces:
- **Array Interface**: Supports probabilistic rounding (PR) on contiguous arrays, providing a simple and flexible interface.
- **Dynamic Interface**: Provides an interface for single vector instructions with **dynamic dispatch** to automatically select the best implementation for the target architecture.
- **Static Interface**: Provides an interface for single vector instructions with **static dispatch**, delivering optimal performance by bypassing architecture selection. This mode is not portable across architectures.

This combination of features makes the library versatile for scientific computing, numerical analysis, and high-performance applications requiring probabilistic rounding.

## Requirements

- clang, clang++
- **bazel** ([install](https://bazel.build/install)) or **bazelisk** ([install](https://github.com/bazelbuild/bazelisk/releases))
  
## Install

```bash
    ./autogen.sh
    ./configure
    ./install.sh
```

## Tests

```bash
    bazel test tests:all
```

## Current status

The library has only been tested on X86-64 architectures for the moment.