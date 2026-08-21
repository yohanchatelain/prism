#ifndef __PRISM_UD_VECTOR_H__
#define __PRISM_UD_VECTOR_H__

#include "hwy/highway.h"
#include "src/prism.h"

#undef PRISM_PR_MODE
#define PRISM_PR_MODE PRISM_UD_MODE

namespace prism::ud::vector::PRISM_DISPATCH {

#include "src/generic_vector.h"

} // namespace prism::ud::vector::PRISM_DISPATCH

#undef PRISM_PR_MODE

#endif // __PRISM_UD_VECTOR_H__