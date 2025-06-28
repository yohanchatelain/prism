#!/usr/bin/env python3
"""
Create sample benchmark data for demonstrating the performance dashboard
"""

import json
import os
import random
import time
from datetime import datetime, timedelta

def create_sample_benchmark_data():
    """Create realistic sample benchmark data for multiple commits"""
    
    # Sample commit hashes (simulated)
    commits = [
        "a1b2c3d4", "e5f6a7b8", "c9d0e1f2", "a3b4c5d6", "e7f8a9b0",
        "c1d2e3f4", "a5b6c7d8", "e9f0a1b2", "c3d4e5f6", "a7b8c9d0"
    ]
    
    # Base performance values (in nanoseconds)
    base_performance = {
        "SR_Add_f": {"base": 120.0, "variance": 10.0},
        "SR_Add_d": {"base": 150.0, "variance": 12.0},
        "SR_Mul_f": {"base": 140.0, "variance": 15.0},
        "SR_Mul_d": {"base": 180.0, "variance": 18.0},
        "UD_Add_f": {"base": 100.0, "variance": 8.0},
        "UD_Add_d": {"base": 125.0, "variance": 10.0},
        "UD_Mul_f": {"base": 115.0, "variance": 12.0},
        "UD_Mul_d": {"base": 145.0, "variance": 14.0},
        "STD_Add_f": {"base": 25.0, "variance": 2.0},
        "STD_Add_d": {"base": 35.0, "variance": 3.0},
        "STD_Mul_f": {"base": 30.0, "variance": 2.5},
        "STD_Mul_d": {"base": 40.0, "variance": 3.5},
    }
    
    # Data sizes to test
    sizes = [1024, 4096, 16384, 65536, 262144, 1048576]
    
    # Create benchmark results directory
    os.makedirs("benchmark_results", exist_ok=True)
    
    # Generate data for multiple commits over time
    base_time = datetime.now() - timedelta(days=30)
    
    for i, commit in enumerate(commits):
        timestamp = (base_time + timedelta(days=i*3)).strftime("%Y-%m-%d_%H-%M-%S")
        
        # Simulate performance evolution over time
        # Introduce some performance regressions and improvements
        performance_factor = 1.0
        if i == 3:  # Regression in commit 4
            performance_factor = 1.25  # 25% slower
        elif i == 5:  # Improvement in commit 6
            performance_factor = 0.85  # 15% faster
        elif i >= 7:  # Gradual improvement
            performance_factor = 0.95 - (i-7) * 0.02
        
        benchmark_data = {
            "commit_hash": commit,
            "timestamp": timestamp,
            "build_config": "Release",
            "cpu_info": "Intel Xeon E5-2686 v4 @ 2.30GHz",
            "benchmarks": {}
        }
        
        # Generate benchmark results for each operation and size
        for operation, perf_data in base_performance.items():
            for size in sizes:
                # Calculate scaling factor based on size (larger = slower per element)
                size_factor = 1.0 + (size / 1000000) * 0.3
                
                # Base time for this configuration
                base_time_ns = perf_data["base"] * size_factor * performance_factor
                
                # Add some realistic variance
                variance = perf_data["variance"] * size_factor
                
                # Generate sample measurements
                measurements = []
                for _ in range(100):  # Simulate 100 measurements
                    measurement = random.normalvariate(base_time_ns, variance)
                    measurements.append(max(0, measurement))  # Ensure positive
                
                # Calculate statistics
                measurements.sort()
                min_time = measurements[0]
                max_time = measurements[-1]
                mean_time = sum(measurements) / len(measurements)
                median_time = measurements[len(measurements)//2]
                p95_time = measurements[int(0.95 * len(measurements))]
                p99_time = measurements[int(0.99 * len(measurements))]
                
                # Standard deviation
                variance_calc = sum((x - mean_time)**2 for x in measurements) / len(measurements)
                stddev_time = variance_calc**0.5
                
                # Throughput (elements per second in millions)
                throughput_mops = (size / (median_time / 1e9)) / 1e6
                
                # Create benchmark entry
                benchmark_name = f"{operation}_{size}"
                benchmark_data["benchmarks"][benchmark_name] = {
                    "operation_name": operation,
                    "data_type": "f" if operation.endswith("_f") else "d",
                    "vector_size": size,
                    "min_time": min_time,
                    "max_time": max_time,
                    "mean_time": mean_time,
                    "median_time": median_time,
                    "stddev_time": stddev_time,
                    "p95_time": p95_time,
                    "p99_time": p99_time,
                    "iterations": 100,
                    "elements_processed": size * 100,
                    "throughput_mops": throughput_mops
                }
        
        # Save benchmark data
        filename = f"benchmark_results/benchmark_{timestamp}.json"
        with open(filename, 'w') as f:
            json.dump(benchmark_data, f, indent=2)
        
        print(f"Created benchmark data for commit {commit}: {filename}")
    
    print(f"\nGenerated {len(commits)} benchmark files in benchmark_results/")
    print("You can now run: python3 tools/performance_dashboard.py")

if __name__ == "__main__":
    create_sample_benchmark_data()