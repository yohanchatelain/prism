#!/bin/bash

# PRISM Performance Analysis Script
# This script runs performance benchmarks and generates interactive reports

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

# Function to check if required tools are installed
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

# Function to run performance benchmarks
run_benchmarks() {
    log_info "Running performance benchmarks..."
    
    cd "$PROJECT_ROOT"
    
    # Create benchmark results directory
    mkdir -p "$BENCHMARK_DIR"
    
    # Run the performance regression tests
    # Note: We use timeout to prevent hanging tests
    timeout 1800 bazel test //tests/performance:performance-regression \
        --test_output=errors \
        --test_timeout=1800 \
        --compilation_mode=opt \
        --copt=-O3 \
        --copt=-march=native \
        --copt=-DNDEBUG \
        || {
            log_error "Performance benchmark failed or timed out"
            return 1
        }
    
    # Copy results to benchmark directory if they exist
    if [ -d "bazel-testlogs/tests/performance/performance-regression" ]; then
        cp -r bazel-testlogs/tests/performance/performance-regression/* "$BENCHMARK_DIR/" 2>/dev/null || true
    fi
    
    log_success "Performance benchmarks completed"
}

# Function to generate performance report
generate_report() {
    log_info "Generating performance dashboard..."
    
    mkdir -p "$REPORT_DIR"
    
    cd "$SCRIPT_DIR"
    
    # Run the performance dashboard generator
    python3 performance_dashboard.py \
        --benchmark-dir "$BENCHMARK_DIR" \
        --output "$REPORT_DIR/performance_dashboard_$(date +%Y%m%d_%H%M%S).html" \
        --regression-threshold 0.1
    
    # Also generate a "latest" version for easy access
    python3 performance_dashboard.py \
        --benchmark-dir "$BENCHMARK_DIR" \
        --output "$REPORT_DIR/latest_performance_report.html" \
        --regression-threshold 0.1
    
    log_success "Performance dashboard generated in $REPORT_DIR"
}

# Function to run commit-based performance analysis
analyze_performance_over_commits() {
    local num_commits=${1:-10}
    
    log_info "Analyzing performance over last $num_commits commits..."
    
    cd "$PROJECT_ROOT"
    
    # Get list of recent commits
    local commits=($(git log --oneline -n "$num_commits" --format="%H"))
    local current_commit=$(git rev-parse HEAD)
    
    for commit in "${commits[@]}"; do
        log_info "Analyzing commit: $commit"
        
        # Checkout commit
        git checkout "$commit" 2>/dev/null || {
            log_warning "Failed to checkout commit $commit, skipping"
            continue
        }
        
        # Run benchmarks for this commit
        run_benchmarks || {
            log_warning "Benchmarks failed for commit $commit"
            continue
        }
        
        log_success "Completed analysis for commit $commit"
    done
    
    # Return to original commit
    git checkout "$current_commit" 2>/dev/null
    
    # Generate comprehensive report
    generate_report
    
    log_success "Multi-commit performance analysis completed"
}

# Function to compare two commits
compare_commits() {
    local commit1="$1"
    local commit2="$2"
    
    if [ -z "$commit1" ] || [ -z "$commit2" ]; then
        log_error "Usage: compare_commits <commit1> <commit2>"
        return 1
    fi
    
    log_info "Comparing performance between commits: $commit1 and $commit2"
    
    cd "$PROJECT_ROOT"
    local current_commit=$(git rev-parse HEAD)
    
    # Benchmark first commit
    log_info "Benchmarking commit: $commit1"
    git checkout "$commit1" 2>/dev/null || {
        log_error "Failed to checkout commit $commit1"
        return 1
    }
    run_benchmarks || {
        log_error "Benchmarks failed for commit $commit1"
        git checkout "$current_commit" 2>/dev/null
        return 1
    }
    
    # Benchmark second commit
    log_info "Benchmarking commit: $commit2"
    git checkout "$commit2" 2>/dev/null || {
        log_error "Failed to checkout commit $commit2"
        git checkout "$current_commit" 2>/dev/null
        return 1
    }
    run_benchmarks || {
        log_error "Benchmarks failed for commit $commit2"
        git checkout "$current_commit" 2>/dev/null
        return 1
    }
    
    # Return to original commit
    git checkout "$current_commit" 2>/dev/null
    
    # Generate comparison report
    generate_report
    
    log_success "Commit comparison completed"
}

# Function to clean up old benchmark data
cleanup_old_data() {
    local days_to_keep=${1:-30}
    
    log_info "Cleaning up benchmark data older than $days_to_keep days..."
    
    if [ -d "$BENCHMARK_DIR" ]; then
        find "$BENCHMARK_DIR" -name "benchmark_*.json" -mtime +$days_to_keep -delete
        log_success "Cleanup completed"
    else
        log_info "No benchmark directory found, nothing to clean"
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
            run_benchmarks
            generate_report
            ;;
        "report")
            check_dependencies
            install_python_deps
            generate_report
            ;;
        "analyze-commits")
            local num_commits=${1:-10}
            check_dependencies
            install_python_deps
            analyze_performance_over_commits "$num_commits"
            ;;
        "compare")
            check_dependencies
            install_python_deps
            compare_commits "$@"
            ;;
        "cleanup")
            local days=${1:-30}
            cleanup_old_data "$days"
            ;;
        "help"|"--help"|"-h"|"")
            cat << EOF
PRISM Performance Analysis Tool

Usage: $0 <command> [options]

Commands:
    run                     Run benchmarks and generate report
    report                  Generate report from existing benchmark data
    analyze-commits [N]     Analyze performance over last N commits (default: 10)
    compare <commit1> <commit2>  Compare performance between two commits
    cleanup [days]          Clean up benchmark data older than N days (default: 30)
    help                    Show this help message

Examples:
    $0 run                              # Run benchmarks and generate report
    $0 analyze-commits 5                # Analyze last 5 commits
    $0 compare HEAD~1 HEAD              # Compare current commit with previous
    $0 cleanup 7                        # Clean up data older than 7 days

Report files are generated in: $REPORT_DIR
Benchmark data is stored in: $BENCHMARK_DIR
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