#ifndef __PRISM_TESTS_PERFORMANCE_BENCHMARK_SUITE_H__
#define __PRISM_TESTS_PERFORMANCE_BENCHMARK_SUITE_H__

#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <map>
#include <fstream>
#include <iomanip>

#ifdef _MSC_VER
#include <intrin.h>
#define PRISM_RDTSC() __rdtsc()
#elif defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#define PRISM_RDTSC() __rdtsc()
#else
#define PRISM_RDTSC() std::chrono::high_resolution_clock::now().time_since_epoch().count()
#endif

namespace prism::tests::performance {

// Cache line size for data alignment
constexpr size_t CACHE_LINE_SIZE = 64;

// Performance measurement statistics
struct BenchmarkStats {
  double min_time = 0.0;
  double max_time = 0.0;
  double mean_time = 0.0;
  double median_time = 0.0;
  double stddev_time = 0.0;
  double p95_time = 0.0;
  double p99_time = 0.0;
  size_t iterations = 0;
  size_t elements_processed = 0;
  double throughput_mops = 0.0; // Million operations per second
  std::string operation_name;
  std::string data_type;
  size_t vector_size = 0;
};

// Results container for regression tracking
struct BenchmarkResult {
  std::string commit_hash;
  std::string timestamp;
  std::string build_config;
  std::string cpu_info;
  std::map<std::string, BenchmarkStats> benchmarks;
};

// High-precision timer class
class HighPrecisionTimer {
private:
  uint64_t start_cycles_;
  std::chrono::high_resolution_clock::time_point start_time_;
  
public:
  void start() {
    start_time_ = std::chrono::high_resolution_clock::now();
    start_cycles_ = PRISM_RDTSC();
  }
  
  double elapsed_nanoseconds() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::nano>(end_time - start_time_).count();
  }
  
  uint64_t elapsed_cycles() const {
    return PRISM_RDTSC() - start_cycles_;
  }
};

// Benchmark configuration
struct BenchmarkConfig {
  size_t warmup_iterations = 1000;
  size_t measurement_iterations = 10000;
  bool use_cpu_cycles = true;
  bool remove_outliers = true;
  double outlier_threshold = 3.0; // Standard deviations
  std::vector<size_t> test_sizes = {1024, 4096, 16384, 65536, 262144};
  std::string output_format = "json"; // json, csv, or both
};

// Main benchmark runner class
class BenchmarkSuite {
private:
  BenchmarkConfig config_;
  BenchmarkResult current_result_;
  
  // Statistical analysis helper
  BenchmarkStats analyze_timings(const std::vector<double>& timings, 
                                const std::string& operation_name,
                                const std::string& data_type,
                                size_t vector_size,
                                size_t elements_per_iteration) {
    BenchmarkStats stats;
    stats.operation_name = operation_name;
    stats.data_type = data_type;
    stats.vector_size = vector_size;
    stats.iterations = timings.size();
    stats.elements_processed = elements_per_iteration * stats.iterations;
    
    if (timings.empty()) return stats;
    
    // Sort for percentile calculations
    auto sorted_timings = timings;
    std::sort(sorted_timings.begin(), sorted_timings.end());
    
    // Remove outliers if requested
    if (config_.remove_outliers && sorted_timings.size() > 10) {
      // Calculate mean and standard deviation
      double mean = std::accumulate(sorted_timings.begin(), sorted_timings.end(), 0.0) / sorted_timings.size();
      double variance = 0.0;
      for (double t : sorted_timings) {
        variance += (t - mean) * (t - mean);
      }
      variance /= sorted_timings.size();
      double stddev = std::sqrt(variance);
      
      // Remove outliers beyond threshold
      auto new_end = std::remove_if(sorted_timings.begin(), sorted_timings.end(),
        [mean, stddev, this](double t) {
          return std::abs(t - mean) > config_.outlier_threshold * stddev;
        });
      sorted_timings.erase(new_end, sorted_timings.end());
    }
    
    // Calculate statistics
    stats.min_time = sorted_timings.front();
    stats.max_time = sorted_timings.back();
    stats.mean_time = std::accumulate(sorted_timings.begin(), sorted_timings.end(), 0.0) / sorted_timings.size();
    stats.median_time = sorted_timings[sorted_timings.size() / 2];
    
    // Standard deviation
    double variance = 0.0;
    for (double t : sorted_timings) {
      variance += (t - stats.mean_time) * (t - stats.mean_time);
    }
    stats.stddev_time = std::sqrt(variance / sorted_timings.size());
    
    // Percentiles
    stats.p95_time = sorted_timings[static_cast<size_t>(0.95 * sorted_timings.size())];
    stats.p99_time = sorted_timings[static_cast<size_t>(0.99 * sorted_timings.size())];
    
    // Throughput calculation (million operations per second)
    stats.throughput_mops = (elements_per_iteration / (stats.median_time / 1e9)) / 1e6;
    
    return stats;
  }
  
  // System information collection
  std::string get_cpu_info() {
    // Simplified CPU info - in real implementation, use cpuid or /proc/cpuinfo
    return "Unknown CPU";
  }
  
  std::string get_commit_hash() {
    // Get git commit hash
    FILE* pipe = popen("git rev-parse HEAD 2>/dev/null", "r");
    if (!pipe) return "unknown";
    
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
    pclose(pipe);
    
    // Remove newline
    if (!result.empty() && result.back() == '\n') {
      result.pop_back();
    }
    return result.empty() ? "unknown" : result;
  }
  
  std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
  }
  
public:
  explicit BenchmarkSuite(const BenchmarkConfig& config = {}) : config_(config) {
    current_result_.commit_hash = get_commit_hash();
    current_result_.timestamp = get_timestamp();
    current_result_.cpu_info = get_cpu_info();
    
    #ifdef NDEBUG
    current_result_.build_config = "Release";
    #else
    current_result_.build_config = "Debug";
    #endif
  }
  
  // Benchmark a function with various data sizes
  template<typename Func, typename T>
  void benchmark_operation(const std::string& operation_name, Func&& func) {
    for (size_t size : config_.test_sizes) {
      benchmark_operation_size<Func, T>(operation_name, std::forward<Func>(func), size);
    }
  }
  
  // Benchmark a function with specific data size
  template<typename Func, typename T>
  void benchmark_operation_size(const std::string& operation_name, Func&& func, size_t size) {
    std::string type_name = typeid(T).name();
    std::string full_name = operation_name + "_" + type_name + "_" + std::to_string(size);
    
    // Allocate aligned memory to avoid alignment penalties
    auto data_a = std::aligned_alloc(CACHE_LINE_SIZE, size * sizeof(T));
    auto data_b = std::aligned_alloc(CACHE_LINE_SIZE, size * sizeof(T));
    auto data_result = std::aligned_alloc(CACHE_LINE_SIZE, size * sizeof(T));
    
    if (!data_a || !data_b || !data_result) {
      throw std::runtime_error("Failed to allocate aligned memory");
    }
    
    T* a = static_cast<T*>(data_a);
    T* b = static_cast<T*>(data_b);
    T* result = static_cast<T*>(data_result);
    
    // Initialize data with meaningful values
    for (size_t i = 0; i < size; i++) {
      a[i] = static_cast<T>(1.0 + i * 1e-7);
      b[i] = static_cast<T>(1e-15 + i * 1e-8);
      result[i] = static_cast<T>(0);
    }
    
    // Warmup phase
    for (size_t i = 0; i < config_.warmup_iterations; i++) {
      func(a, b, result, size);
      // Prevent optimization
      asm volatile("" : : "m"(result[0]) : "memory");
    }
    
    // Measurement phase
    std::vector<double> timings;
    timings.reserve(config_.measurement_iterations);
    
    HighPrecisionTimer timer;
    
    for (size_t i = 0; i < config_.measurement_iterations; i++) {
      timer.start();
      func(a, b, result, size);
      
      // Prevent compiler optimization
      asm volatile("" : : "m"(result[0]) : "memory");
      
      if (config_.use_cpu_cycles) {
        timings.push_back(static_cast<double>(timer.elapsed_cycles()));
      } else {
        timings.push_back(timer.elapsed_nanoseconds());
      }
    }
    
    // Analyze results
    BenchmarkStats stats = analyze_timings(timings, operation_name, type_name, size, size);
    current_result_.benchmarks[full_name] = stats;
    
    // Cleanup
    std::free(data_a);
    std::free(data_b);
    std::free(data_result);
  }
  
  // Save results to file
  void save_results(const std::string& output_dir = "benchmark_results") {
    // Create directory if it doesn't exist
    system(("mkdir -p " + output_dir).c_str());
    
    std::string filename = output_dir + "/benchmark_" + current_result_.timestamp + ".json";
    std::ofstream file(filename);
    
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open output file: " + filename);
    }
    
    // Write JSON format
    file << "{\n";
    file << "  \"commit_hash\": \"" << current_result_.commit_hash << "\",\n";
    file << "  \"timestamp\": \"" << current_result_.timestamp << "\",\n";
    file << "  \"build_config\": \"" << current_result_.build_config << "\",\n";
    file << "  \"cpu_info\": \"" << current_result_.cpu_info << "\",\n";
    file << "  \"benchmarks\": {\n";
    
    bool first = true;
    for (const auto& [name, stats] : current_result_.benchmarks) {
      if (!first) file << ",\n";
      first = false;
      
      file << "    \"" << name << "\": {\n";
      file << "      \"operation_name\": \"" << stats.operation_name << "\",\n";
      file << "      \"data_type\": \"" << stats.data_type << "\",\n";
      file << "      \"vector_size\": " << stats.vector_size << ",\n";
      file << "      \"min_time\": " << stats.min_time << ",\n";
      file << "      \"max_time\": " << stats.max_time << ",\n";
      file << "      \"mean_time\": " << stats.mean_time << ",\n";
      file << "      \"median_time\": " << stats.median_time << ",\n";
      file << "      \"stddev_time\": " << stats.stddev_time << ",\n";
      file << "      \"p95_time\": " << stats.p95_time << ",\n";
      file << "      \"p99_time\": " << stats.p99_time << ",\n";
      file << "      \"iterations\": " << stats.iterations << ",\n";
      file << "      \"elements_processed\": " << stats.elements_processed << ",\n";
      file << "      \"throughput_mops\": " << stats.throughput_mops << "\n";
      file << "    }";
    }
    
    file << "\n  }\n";
    file << "}\n";
    
    file.close();
    std::cout << "Benchmark results saved to: " << filename << std::endl;
  }
  
  // Print summary to console
  void print_summary() const {
    std::cout << "\n=== Benchmark Summary ===" << std::endl;
    std::cout << "Commit: " << current_result_.commit_hash << std::endl;
    std::cout << "Build: " << current_result_.build_config << std::endl;
    std::cout << "Timestamp: " << current_result_.timestamp << std::endl;
    std::cout << "\nResults:" << std::endl;
    
    std::cout << std::left << std::setw(30) << "Operation"
              << std::setw(12) << "Size"
              << std::setw(15) << "Median (ns)"
              << std::setw(15) << "Throughput"
              << std::setw(12) << "StdDev"
              << std::endl;
    std::cout << std::string(84, '-') << std::endl;
    
    for (const auto& [name, stats] : current_result_.benchmarks) {
      std::cout << std::left << std::setw(30) << stats.operation_name
                << std::setw(12) << stats.vector_size
                << std::setw(15) << std::fixed << std::setprecision(2) << stats.median_time
                << std::setw(15) << std::fixed << std::setprecision(2) << stats.throughput_mops << " MOPS"
                << std::setw(12) << std::fixed << std::setprecision(2) << (stats.stddev_time / stats.mean_time * 100) << "%"
                << std::endl;
    }
  }
  
  // Get current results for external processing
  const BenchmarkResult& get_results() const {
    return current_result_;
  }
};

} // namespace prism::tests::performance

#endif // __PRISM_TESTS_PERFORMANCE_BENCHMARK_SUITE_H__