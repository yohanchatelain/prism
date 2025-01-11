
// Generates code for every target that this compiler can support.
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "src/sr_vector_static.cpp" // this file
#include "hwy/foreach_target.h" // must come before highway.h

#include "hwy/highway.h"
#include "src/prism.h"
#include "src/sr_vector-inl.h"
#include "src/sr_vector.h" // IWYU pragma: keep

/* static dispatch */
#undef PRISM_DISPATCH
#define PRISM_DISPATCH static_dispatch
#undef PRISM_PR_MODE_NAMESPACE
#define PRISM_PR_MODE_NAMESPACE prism::sr::vector
#undef PRISM_PR_MODE
#define PRISM_PR_MODE PRISM_SR_MODE

#include "src/generic_vector-inl.h"