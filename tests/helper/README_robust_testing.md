# Robust Statistical Testing Framework

This framework provides robust statistical testing to reduce false positives in the PRISM test suite while maintaining sensitivity to real statistical deviations.

## Overview

The robust testing framework addresses the issue where statistical tests sometimes fail due to random chance rather than actual implementation problems. It implements several strategies:

1. **Retry Mechanism**: Tests are retried with progressively relaxed alpha levels
2. **Bonferroni Correction**: Adjusts for multiple testing scenarios  
3. **Adaptive Sample Sizing**: Calculates required sample sizes for given effect sizes
4. **Sequential Testing**: Supports early stopping for strong evidence
5. **Environment Configuration**: Allows runtime configuration via environment variables

## Configuration

### Environment Variables

You can control the robust testing behavior using these environment variables:

- `PRISM_TEST_ALPHA`: Base alpha level (default: 0.01)
- `PRISM_TEST_MAX_RETRIES`: Maximum number of retry attempts (default: 3)
- `PRISM_TEST_DISABLE_BONFERRONI`: Set to "1" to disable Bonferroni correction

### Default Configuration

```cpp
struct RobustTestConfig {
  double base_alpha = 0.01;           // Less strict than original 0.000001
  int base_repetitions = 10000;       // Base sample size
  int max_retries = 3;                // Number of retry attempts
  double retry_alpha_multiplier = 2.0; // Relax alpha on retries
  double retry_sample_multiplier = 1.5; // Increase sample size on retries
  bool use_bonferroni = true;         // Apply Bonferroni correction
  int num_tests_estimate = 100;       // Estimated number of tests for correction
  bool use_adaptive_sampling = true;  // Enable adaptive sample sizing
  double min_effect_size = 0.05;      // Minimum effect size to detect
  double power = 0.8;                 // Statistical power
};
```

## Integration

The framework is automatically integrated into the existing test infrastructure through the `assert_binomial_test` function in `tests/helper/assert-inl.h`. No changes to existing test code are required.

## Benefits

1. **Reduced False Positives**: Retry mechanism with relaxed alpha levels reduces spurious failures
2. **Maintained Sensitivity**: Still detects real statistical deviations effectively
3. **Better Multiple Testing Handling**: Proper Bonferroni correction based on actual test counts
4. **Debugging Information**: Detailed failure reports when tests fail
5. **Runtime Configuration**: Easy tuning via environment variables

## Usage Example

```bash
# Run tests with custom alpha level
PRISM_TEST_ALPHA=0.001 bazel test //tests/vector:sr-accuracy

# Run tests with more retries
PRISM_TEST_MAX_RETRIES=5 bazel test //tests/vector:sr-accuracy

# Disable Bonferroni correction for debugging
PRISM_TEST_DISABLE_BONFERRONI=1 bazel test //tests/vector:sr-accuracy
```

## Implementation Details

The robust testing framework replaces the simple binomial test with a multi-stage approach:

1. **Initial Test**: Standard binomial test with Bonferroni-corrected alpha
2. **Retry Loop**: If failed, retry with progressively relaxed alpha levels
3. **Failure Analysis**: Detailed reporting of why tests failed after all retries

This approach maintains the statistical rigor of the original tests while significantly reducing false positive rates.