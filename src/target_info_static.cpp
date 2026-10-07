#include "hwy/highway.h"
#include "src/target_info.h"

namespace prism::target_info {

auto static_target() -> std::string {
  return hwy::TargetName(HWY_STATIC_TARGET);
}

} // namespace prism::target_info
