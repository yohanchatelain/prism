#ifndef __PRISM_SR_SCALAR_H__
#define __PRISM_SR_SCALAR_H__


namespace prism::sr::scalar {

namespace dynamic_dispatch {

/* dynamic dispatch */
float addf32(float a, float b);
float subf32(float a, float b);
float mulf32(float a, float b);
float divf32(float a, float b);
float sqrtf32(float a);
float fmaf32(float a, float b, float c);

double addf64(double a, double b);
double subf64(double a, double b);
double mulf64(double a, double b);
double divf64(double a, double b);
double sqrtf64(double a);
double fmaf64(double a, double b, double c);

} // namespace dynamic_dispatch

namespace static_dispatch {

/* static dispatch */
float addf32(float a, float b);
float subf32(float a, float b);
float mulf32(float a, float b);
float divf32(float a, float b);
float sqrtf32(float a);
float fmaf32(float a, float b, float c);

double addf64(double a, double b);
double subf64(double a, double b);
double mulf64(double a, double b);
double divf64(double a, double b);
double sqrtf64(double a);
double fmaf64(double a, double b, double c);

} // namespace static_dispatch

} // namespace prism::sr::scalar

#endif /* __PRISM_SR_SCALAR_H__ */