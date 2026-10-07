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

## Binary releases

Linux x86-64 binaries are attached to each [GitHub release](https://github.com/verificarlo/prism/releases). Choose the highest architecture level supported by every machine that will run the static-dispatch library and the LLVM major version used by your LLVM IR pipeline:

| Architecture component | Minimum CPU features |
| --- | --- |
| `x86-64` | Baseline x86-64 |
| `x86-64-v2` | SSE3, SSSE3, SSE4.1, SSE4.2, and POPCNT |
| `x86-64-v3` | AVX, AVX2, BMI1/2, F16C, FMA, and related features |
| `x86-64-v4` | AVX-512 foundation and the standard v4 extensions |

For example, set the desired release and LLVM versions and install the baseline package under `/usr/local`:

```bash
VERSION=X.Y.Z
LLVM_MAJOR=18
curl -LO "https://github.com/verificarlo/prism/releases/download/v${VERSION}/prism-${VERSION}-linux-x86-64-llvm${LLVM_MAJOR}.tar.gz"
sudo tar -C /usr/local --strip-components=1 -xzf "prism-${VERSION}-linux-x86-64-llvm${LLVM_MAJOR}.tar.gz"
sudo ldconfig
```

Each release also includes `SHA256SUMS`. Archives are built on Ubuntu 22.04 for LLVM 17, 18, 19, and 20. They contain shared and static libraries, public headers, and generated LLVM IR files; select the matching `llvmN` archive when consuming those IR files. Package documentation and build metadata are installed under `share/doc/prism`. The dynamic-dispatch library selects a supported vector target at runtime; the static-dispatch library requires the CPU level named by the archive.

## Requirements

- clang, clang++
- parallel ([install](https://www.gnu.org/software/parallel/))
- **bazelisk** ([install](https://github.com/bazelbuild/bazelisk/releases))

## Build and install from source

The default build requests `-march=native` and falls back to `-mtune=native` when the compiler does not support it. Pass `--with-arch` to build the static-dispatch library for a specific CPU level.

```bash
./autogen.sh
./configure
# ./configure --with-arch=x86-64-v3
make
make install
```

## Tests

```bash
bazel test tests:all
```

## Current status

The library is tested on x86-64. Arm (aarch64) support is **experimental**:

- NEON targets are built by default.
- SVE targets are opt-in with `./configure --enable-experimental-sve` and need clang >= 22 (Highway generates no SVE code with older clang; clang 21 only supports `SVE2_128`).
- `-march=native` falls back to a generic armv8 CPU when clang does not know the host CPU; pass `--with-arch` (e.g. `armv9-a+sve2`) in that case.

`make target-info` prints the Highway targets the static and dynamic libraries were built for.

## Publications

If you use PRISM, please cite the relevant papers (see also [`CITATION.cff`](CITATION.cff)):

- I. González-Pepe, H. Akhaddar, T. Glatard, Y. Chatelain. *Fuzzy PyTorch: Rapid Numerical Variability Evaluation for Deep Learning Models*. Transactions on Machine Learning Research, 2026. [arXiv:2605.25991](https://arxiv.org/abs/2605.25991)
- Y. Chatelain, P. de Oliveira Castro. *Stochastic Rounding in Low-Precision Transformer Inference: A Variable-Precision Emulation Study of a Small GPT-2*. 2026. [arXiv:2610.01889](https://arxiv.org/abs/2610.01889)
