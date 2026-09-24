#ifndef __PRISM_SR_HW_H__
#define __PRISM_SR_HW_H__

#include "hwy/highway.h"
#include "src/prism.h"

#ifdef PRISM_IDE
#undef HWY_MAX_BYTES
#define HWY_MAX_BYTES UINT32_MAX
#endif

#undef PRISM_PR_MODE
#define PRISM_PR_MODE PRISM_SR_MODE

namespace prism::sr::vector::PRISM_DISPATCH {

#include "src/generic_vector.h"

} // namespace prism::sr::vector::PRISM_DISPATCH

#undef PRISM_PR_MODE

#endif // __PRISM_SR_HW_H__