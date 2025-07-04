#!/bin/bash

# PRISM Performance Analysis Script - Using Existing Working Tests
# This script uses the existing working performance tests to generate real benchmark data

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BENCHMARK_DIR="$PROJECT_ROOT/benchmark_results"
REPORT_DIR="$PROJECT_ROOT/performance_reports"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to extract performance data from existing test output
extract_performance_data() {
    local test_output="$1"
    local operation_name="$2"
    local data_type="$3"
    
    # Parse the output to extract timing information
    # The existing tests output format: [size] mean ± std [min - max] (iterations)
    echo "$test_output" | grep -E '\[[0-9]+\]' | while read line; do
        # Extract values using regex
        if [[ $line =~ \[([0-9]+)\].*([0-9]+\.[0-9]+e[+-][0-9]+).*±.*([0-9]+\.[0-9]+e[+-][0-9]+).*\[([0-9]+\.[0-9]+e[+-][0-9]+).*-.*([0-9]+\.[0-9]+e[+-][0-9]+)\].*\(([0-9]+)\) ]]; then
            local size=${BASH_REMATCH[1]}
            local mean=${BASH_REMATCH[2]}
            local std=${BASH_REMATCH[3]}
            local min_time=${BASH_REMATCH[4]}
            local max_time=${BASH_REMATCH[5]}
            local iterations=${BASH_REMATCH[6]}
            
            # Convert scientific notation to regular numbers for processing
            local mean_ns=$(python3 -c "print(float('$mean') * 1e9)")
            local std_ns=$(python3 -c "print(float('$std') * 1e9)")
            local min_ns=$(python3 -c "print(float('$min_time') * 1e9)")
            local max_ns=$(python3 -c "print(float('$max_time') * 1e9)")
            
            # Calculate throughput (elements per second)
            local throughput_mops=$(python3 -c "print(($size / float('$mean')) / 1e6)")
            
            echo "    \"${operation_name}_${data_type}_${size}\": {"
            echo "      \"operation_name\": \"${operation_name}\","
            echo "      \"data_type\": \"${data_type}\","
            echo "      \"vector_size\": ${size},"
            echo "      \"min_time\": ${min_ns},"
            echo "      \"max_time\": ${max_ns},"
            echo "      \"mean_time\": ${mean_ns},"
            echo "      \"median_time\": ${mean_ns},"
            echo "      \"stddev_time\": ${std_ns},"
            echo "      \"p95_time\": $(python3 -c "print(float('$mean_ns') + 1.645 * float('$std_ns'))"),"
            echo "      \"p99_time\": $(python3 -c "print(float('$mean_ns') + 2.326 * float('$std_ns'))"),"
            echo "      \"iterations\": ${iterations},"
            echo "      \"elements_processed\": $((size * iterations)),"
            echo "      \"throughput_mops\": ${throughput_mops}"
            echo "    },"
        fi
    done
}

# Function to run existing performance tests and capture data
run_existing_perf_tests() {
    log_info "Running existing PRISM performance tests..."
    
    cd "$PROJECT_ROOT"
    mkdir -p "$BENCHMARK_DIR"
    
    # Get current commit info
    local commit_hash=$(git rev-parse HEAD 2>/dev/null || echo "unknown")
    local timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
    local output_file="$BENCHMARK_DIR/benchmark_data_${timestamp}.json"
    local raw_benchmark_output_file="$BENCHMARK_DIR/raw_benchmark_output_${timestamp}.txt"

    log_info "Running Bazel performance tests..."
    echo "---START_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"
    echo "test_name: sr-perf-dynamic" >> "$raw_benchmark_output_file"
    echo "precision: float" >> "$raw_benchmark_output_file"
    bazel test //tests/vector:sr-perf-dynamic --test_output=all 2>&1 | grep -E '^\[[0-9]+\s*\]|Measure function' >> "$raw_benchmark_output_file" || true
    echo "---END_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"

    echo "---START_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"
    echo "test_name: ud-perf-dynamic" >> "$raw_benchmark_output_file"
    echo "precision: float" >> "$raw_benchmark_output_file"
    bazel test //tests/vector:ud-perf-dynamic --test_output=all 2>&1 | grep -E '^[0-9]+\s*\]|Measure function' >> "$raw_benchmark_output_file" || true
    echo "---END_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"

    echo "---START_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"
    echo "test_name: sr-perf-static" >> "$raw_benchmark_output_file"
    echo "precision: float" >> "$raw_benchmark_output_file"
    bazel test //tests/vector:sr-perf-static --test_output=all 2>&1 | grep -E '^\[[0-9]+\s*\]|Measure function' >> "$raw_benchmark_output_file" || true
    echo "---END_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"

    echo "---START_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"
    echo "test_name: ud-perf-static" >> "$raw_benchmark_output_file"
    echo "precision: float" >> "$raw_benchmark_output_file"
    bazel test //tests/vector:ud-perf-static --test_output=all 2>&1 | grep -E '^\[[0-9]+\s*\]|Measure function' >> "$raw_benchmark_output_file" || true
    echo "---END_BENCHMARK_OUTPUT---" >> "$raw_benchmark_output_file"

    log_success "Bazel performance tests completed. Raw output saved to $raw_benchmark_output_file"

    # Create benchmark data file
    # Pass the raw output file to the Python script for processing
    python3 "$SCRIPT_DIR/performance_dashboard.py" \
        --benchmark-dir "$BENCHMARK_DIR" \
        --output "$output_file" \
        --raw-output-file "$raw_benchmark_output_file" \
        --commit-hash "${commit_hash:0:8}" \
        --timestamp "$timestamp" \
        --cpu-info "$(lscpu | grep 'Model name' | cut -d':' -f2 | xargs || echo 'Unknown CPU')"

    rm "$raw_benchmark_output_file"

    log_success "Performance data saved to: $output_file"
    echo "$output_file"
}

# Function to generate performance report


# Function to generate performance report


# Function to check dependencies
check_dependencies() {
    log_info "Checking dependencies..."
    
    if ! command -v bazel &> /dev/null; then
        log_error "Bazel is not installed or not in PATH"
        exit 1
    fi
    
    if ! command -v python3 &> /dev/null; then
        log_error "Python3 is not installed or not in PATH"
        exit 1
    fi
    
    log_success "Dependencies check passed"
}

# Function to install Python dependencies
install_python_deps() {
    log_info "Installing Python dependencies..."
    
    if [ -f "$SCRIPT_DIR/requirements.txt" ]; then
        python3 -m pip install -r "$SCRIPT_DIR/requirements.txt" --user
        log_success "Python dependencies installed"
    else
        log_warning "requirements.txt not found, skipping Python dependency installation"
    fi
}

# Main function
main() {
    local command="$1"
    shift
    
    case "$command" in
        "run")
            check_dependencies
            install_python_deps
            run_existing_perf_tests
            ;;
        "report")
            check_dependencies
            install_python_deps
            generate_report
            ;;
        "help"|"--help"|"-h"|"")
            cat << EOF
PRISM Performance Analysis Tool (Using Existing Tests)

Usage: $0 <command> [options]

Commands:
    run                     Run existing performance tests and generate report
    report                  Generate report from existing benchmark data
    help                    Show this help message

Examples:
    $0 run                              # Run existing perf tests and generate report
    $0 report                           # Generate report from existing data

Report files are generated in: $REPORT_DIR
Benchmark data is stored in: $BENCHMARK_DIR

Note: This version uses the existing working performance tests (sr-perf-dynamic, sr-perf-static)
      to avoid the build issues with the new comprehensive test suite.
EOF
            ;;
        *)
            log_error "Unknown command: $command"
            log_info "Use '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"