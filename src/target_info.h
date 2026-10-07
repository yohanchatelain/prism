#ifndef __PRISM_TARGET_INFO_H__
#define __PRISM_TARGET_INFO_H__

#include <string>

// Highway targets of the PRISM libraries, as built (see prism-target-info).
namespace prism::target_info {

// Target of the static dispatch library.
auto static_target() -> std::string;

// Targets compiled into the dynamic dispatch library.
auto dynamic_targets() -> std::string;

// Target the dynamic dispatch library selects on this CPU.
auto dynamic_dispatch_target() -> std::string;

} // namespace prism::target_info

#endif // __PRISM_TARGET_INFO_H__
