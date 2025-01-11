#ifndef __PRISM_SR_HW_H__
#define __PRISM_SR_HW_H__

#include "hwy/highway.h"

#ifdef PRISM_IDE
#undef HWY_MAX_BYTES
#define HWY_MAX_BYTES UINT32_MAX
#endif

namespace prism::sr::vector::PRISM_DISPATCH {

#include "src/generic_vector.h"

} // namespace prism::sr::vector::PRISM_DISPATCH

#endif // __PRISM_SR_HW_H__