#include <iostream>
#include <set>

#if defined(PRISM_TARGET_UTILS_H_) == defined(HWY_TARGET_TOGGLE) // NOLINT
#ifdef PRISM_TARGET_UTILS_H_
#undef PRISM_TARGET_UTILS_H_
#else
#define PRISM_TARGET_UTILS_H_
#endif

#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism {

namespace HWY_NAMESPACE {

inline std::set<std::string> GetTargetsAsString() {
  const auto targets = hwy::SupportedAndGeneratedTargets();
  std::set<std::string> result;
  for (auto target : targets) {
    result.insert(hwy::TargetName(target));
  }
  return result;
}

HWY_API bool isCurrentTargetSupported() {
  const auto targets = GetTargetsAsString();
  const auto current_target = hwy::TargetName(HWY_TARGET);
#ifdef PRISM_DEBUG
  fprintf(stderr, "[PRISM Debug] Current target: %s is supported: %d\n",
          current_target, targets.find(current_target) != targets.end());
#endif
  return targets.find(current_target) != targets.end();
}

} // namespace HWY_NAMESPACE

} // namespace prism
HWY_AFTER_NAMESPACE();

#endif // PRISM_TARGET_UTILS_H_