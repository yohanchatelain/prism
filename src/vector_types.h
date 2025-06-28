#ifndef __PRISM_VECTOR_TYPES_H__
#define __PRISM_VECTOR_TYPES_H__

// Cache line size for optimal alignment
#ifndef PRISM_CACHE_LINE_SIZE
#define PRISM_CACHE_LINE_SIZE 64
#endif

typedef float f32x2_v __attribute__((vector_size(8)));
typedef double f64x2_v __attribute__((vector_size(16)));
typedef float f32x4_v __attribute__((vector_size(16)));
typedef double f64x4_v __attribute__((vector_size(32)));
typedef float f32x8_v __attribute__((vector_size(32)));
typedef double f64x8_v __attribute__((vector_size(64)));
typedef float f32x16_v __attribute__((vector_size(64)));
typedef double f64x16_v __attribute__((vector_size(128)));

union f32x2_u {
  f32x2_v vector;
  alignas(8) float array[2];
};
union f64x2_u {
  f64x2_v vector;
  alignas(16) double array[2];
};
union f32x4_u {
  f32x4_v vector;
  alignas(16) float array[4];
};

union f64x4_u {
  f64x4_v vector;
  alignas(32) double array[4];
};
union f32x8_u {
  f32x8_v vector;
  alignas(PRISM_CACHE_LINE_SIZE) float array[8];
};
union f64x8_u {
  f64x8_v vector;
  alignas(PRISM_CACHE_LINE_SIZE) double array[8];
};
union f32x16_u {
  f32x16_v vector;
  alignas(PRISM_CACHE_LINE_SIZE) float array[16];
};
union f64x16_u {
  f64x16_v vector;
  alignas(PRISM_CACHE_LINE_SIZE * 2) double array[16];
};

#endif // __PRISM_VECTOR_TYPES_H__