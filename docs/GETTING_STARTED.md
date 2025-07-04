# Getting Started with PRISM

This guide will help you get PRISM up and running quickly, from installation to your first performance analysis.

## 📋 Prerequisites

### System Requirements
- **Operating System**: Linux (Ubuntu 18.04+), macOS (10.15+), Windows (WSL2)
- **Architecture**: x86-64 (primary support), ARM64 (experimental)
- **Memory**: 4GB RAM minimum, 8GB recommended
- **Storage**: 2GB free space for build artifacts

### Required Software
- **Compiler**: Clang 12+ (recommended) or GCC 9+
- **Build System**: Bazelisk (latest version)
- **Python**: 3.7+ (for performance analysis tools)
- **Git**: For version control and performance tracking

## 🚀 Quick Installation

### 1. Install Dependencies

#### Ubuntu/Debian
```bash
# Install essential build tools
sudo apt update
sudo apt install -y clang-12 libc++-dev python3 python3-pip git

# Install Bazelisk
curl -L https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64 -o bazelisk
chmod +x bazelisk && sudo mv bazelisk /usr/local/bin/bazel
```

#### macOS
```bash
# Install via Homebrew
brew install llvm bazel python3 git

# Add LLVM to PATH
echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

#### Windows (WSL2)
```bash
# Install WSL2 Ubuntu first, then follow Ubuntu instructions
wsl --install -d Ubuntu
# ... follow Ubuntu steps above
```

### 2. Clone and Build PRISM

```bash
# Clone the repository
git clone <repository-url>
cd prism

# Configure with Clang (recommended for performance)
./autogen.sh
./configure --with-prefix=$(llvm-config-12 --prefix)

# Build with parallel compilation
make -j$(nproc)

# Install (optional)
sudo make install
```

### 3. Install Performance Analysis Tools

```bash
# Install Python dependencies for dashboard
pip3 install -r tools/requirements.txt

# Verify installation
python3 tools/performance_dashboard.py --help
```

## ✅ Verify Installation

### 1. Run Basic Tests
```bash
# Test core functionality
bazel test //tests/vector:sr-accuracy
bazel test //tests/vector:ud-accuracy

# Expected output: All tests pass
```

### 2. Run Performance Tests
```bash
# Test performance framework
bazel test //tests/vector:sr-perf-dynamic

# Expected output: Performance test completes in 10-15 seconds
```

### 3. Generate Sample Performance Report
```bash
# Create sample data
python3 tools/create_sample_data.py

# Generate dashboard
./tools/run_performance_analysis.sh report

# View results
open performance_reports/latest_performance_report.html
```

## 🎯 Your First PRISM Program

### 1. Simple Stochastic Rounding Example

Create `hello_prism.cpp`:
```cpp
#include <iostream>
#include <vector>
#include "src/sr_vector.h"

int main() {
    using namespace prism::sr::vector::PRISM_DISPATCH;
    
    // Create test data
    constexpr size_t SIZE = 1000;
    std::vector<float> a(SIZE, 1.0f);
    std::vector<float> b(SIZE, 1e-7f);  // Small value for stochastic effect
    std::vector<float> result(SIZE);
    
    // Perform stochastic rounding addition
    variable::addf32(a.data(), b.data(), result.data(), SIZE);
    
    // Display some results
    std::cout << "Stochastic Rounding Results (first 5 elements):\n";
    for (int i = 0; i < 5; i++) {
        std::cout << "a[" << i << "] + b[" << i << "] = " << result[i] << "\n";
    }
    
    return 0;
}
```

### 2. Build and Run

```bash
# Compile
clang++ -std=c++17 -I. hello_prism.cpp -L. -lprism -o hello_prism

# Run
./hello_prism
```

Expected output:
```
Stochastic Rounding Results (first 5 elements):
a[0] + b[0] = 1.0000001
a[1] + b[1] = 1
a[2] + b[2] = 1.0000001
a[3] + b[3] = 1
a[4] + b[4] = 1.0000001
```

## 📊 Your First Performance Analysis

### 1. Create Performance Benchmark

Create `benchmark_example.cpp`:
```cpp
#include <chrono>
#include <iostream>
#include "src/sr_vector.h"

void benchmark_operation(size_t size, int iterations) {
    using namespace prism::sr::vector::PRISM_DISPATCH;
    
    std::vector<float> a(size, 1.0f);
    std::vector<float> b(size, 1e-6f);
    std::vector<float> result(size);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        variable::addf32(a.data(), b.data(), result.data(), size);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    double avg_time = static_cast<double>(duration.count()) / iterations;
    double throughput = (size / (avg_time / 1e9)) / 1e6;  // MOPS
    
    std::cout << "Size: " << size 
              << ", Avg Time: " << avg_time << " ns"
              << ", Throughput: " << throughput << " MOPS\n";
}

int main() {
    std::cout << "PRISM Performance Benchmark\n";
    std::cout << "============================\n";
    
    const std::vector<size_t> sizes = {1024, 4096, 16384, 65536};
    const int iterations = 1000;
    
    for (size_t size : sizes) {
        benchmark_operation(size, iterations);
    }
    
    return 0;
}
```

### 2. Run Benchmark

```bash
# Compile with optimizations
clang++ -std=c++17 -O3 -march=native -I. benchmark_example.cpp -L. -lprism -o benchmark

# Run
./benchmark
```

Expected output:
```
PRISM Performance Benchmark
============================
Size: 1024, Avg Time: 122450 ns, Throughput: 8.36 MOPS
Size: 4096, Avg Time: 489123 ns, Throughput: 8.38 MOPS
Size: 16384, Avg Time: 1956789 ns, Throughput: 8.37 MOPS
Size: 65536, Avg Time: 7834567 ns, Throughput: 8.36 MOPS
```

## 🔧 Common Issues and Solutions

### Build Issues

#### Issue: "clang not found"
```bash
# Solution: Install or specify path
sudo apt install clang-12
# Or specify full path in configure
./configure --with-prefix=/usr/lib/llvm-12
```

#### Issue: "bazel: command not found"
```bash
# Solution: Install Bazelisk
curl -L https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64 -o bazelisk
chmod +x bazelisk && sudo mv bazelisk /usr/local/bin/bazel
```

#### Issue: "Python module not found"
```bash
# Solution: Install required packages
pip3 install pandas plotly numpy
# Or use requirements file
pip3 install -r tools/requirements.txt
```

### Performance Issues

#### Issue: "Performance dashboard fails"
```bash
# Solution: Use alternative approach
./tools/run_existing_perf_tests.sh run
```

#### Issue: "Tests timeout"
```bash
# Solution: Reduce test size for development
export PRISM_TEST_REPETITIONS=1000  # Instead of default 10000
bazel test //tests/vector:sr-accuracy
```

### Runtime Issues

#### Issue: "Segmentation fault"
```bash
# Solution: Enable debug mode
#define PRISM_DEBUG
# Recompile and run with gdb
gdb ./your_program
```

## 📚 Next Steps

### Learn the API
1. **Read the [API Documentation](../README.md#-api-documentation)**
2. **Explore [Usage Examples](../README.md#-usage-examples)**
3. **Study the [Performance Optimization Guide](../PERFORMANCE_OPTIMIZATION_REPORT.md)**

### Performance Analysis
1. **Set up continuous monitoring**: `./tools/run_performance_analysis.sh analyze-commits 10`
2. **Create custom benchmarks** using the performance framework
3. **Integrate with CI/CD** for automated regression detection

### Advanced Features
1. **Experiment with different dispatch modes** (static vs dynamic)
2. **Try different random number generation modes** (full-bits vs partial-bits)
3. **Explore architecture-specific optimizations**

### Community
1. **Report issues** with performance analysis data
2. **Share benchmarks** and optimization results
3. **Contribute improvements** following the [Contributing Guide](../README.md#-contributing)

## 🎯 Success Checklist

By the end of this guide, you should have:

- ✅ **Installed PRISM** with all dependencies
- ✅ **Verified functionality** with passing tests
- ✅ **Generated performance reports** with interactive dashboard
- ✅ **Written your first PRISM program** using stochastic rounding
- ✅ **Created custom benchmarks** measuring performance
- ✅ **Understood the basics** of performance analysis tools

**Welcome to the PRISM community! You're now ready to leverage high-performance probabilistic rounding in your applications.**