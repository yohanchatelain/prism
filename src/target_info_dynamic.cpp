#include <cstdint>

#include "hwy/highway.h"
#include "hwy/targets.h"
#include "src/target_info.h"

namespace prism::target_info {

auto dynamic_targets() -> std::string {
  std::string names;
  for (int64_t targets = HWY_TARGETS; targets != 0; targets &= targets - 1) {
    if (!names.empty()) {
      names += " ";
    }
    names += hwy::TargetName(targets & -targets);
  }
  return names;
}

auto dynamic_dispatch_target() -> std::string {
  // Lower bits are better targets.
  const int64_t usable = hwy::SupportedTargets() & HWY_TARGETS;
  return usable != 0 ? hwy::TargetName(usable & -usable) : "none";
}

} // namespace prism::target_info
