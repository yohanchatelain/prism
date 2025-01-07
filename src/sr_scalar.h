#ifndef __PRISM_SR_SCALAR_H__
#define __PRISM_SR_SCALAR_H__

namespace prism::sr::scalar {

namespace dynamic_dispatch {

/* dynamic dispatch */
auto addf32(float a, float b) -> float;
auto subf32(float a, float b) -> float;
auto mulf32(float a, float b) -> float;
auto divf32(float a, float b) -> float;
auto sqrtf32(float a) -> float;
auto fmaf32(float a, float b, float c) -> float;

auto addf64(double a, double b) -> double;
auto subf64(double a, double b) -> double;
auto mulf64(double a, double b) -> double;
auto divf64(double a, double b) -> double;
auto sqrtf64(double a) -> double;
auto fmaf64(double a, double b, double c) -> double;

} // namespace dynamic_dispatch

namespace static_dispatch {

/* static dispatch */
auto addf32(float a, float b) -> float;
auto subf32(float a, float b) -> float;
auto mulf32(float a, float b) -> float;
auto divf32(float a, float b) -> float;
auto sqrtf32(float a) -> float;
auto fmaf32(float a, float b, float c) -> float;

auto addf64(double a, double b) -> double;
auto subf64(double a, double b) -> double;
auto mulf64(double a, double b) -> double;
auto divf64(double a, double b) -> double;
auto sqrtf64(double a) -> double;
auto fmaf64(double a, double b, double c) -> double;

} // namespace static_dispatch

} // namespace prism::sr::scalar

#endif /* __PRISM_SR_SCALAR_H__ */