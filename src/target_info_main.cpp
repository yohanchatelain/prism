#include <cstdio>

#include "src/target_info.h"

namespace ti = prism::target_info;

auto main() -> int {
  std::printf("PRISM targets\n");
  std::printf("  static library:            %s\n", ti::static_target().c_str());
  std::printf("  dynamic library, compiled: %s\n",
              ti::dynamic_targets().c_str());
  std::printf("  dynamic library, selected: %s\n",
              ti::dynamic_dispatch_target().c_str());
  return 0;
}
