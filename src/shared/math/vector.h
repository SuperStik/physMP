#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H 1

#include <math/misc.h>

#ifdef __APPLE__
# include <simd/math.h>
#endif /* __APPLE__ */

#define gvec(type, elems) type __attribute__((vector_size(sizeof(type)*elems)))

#ifndef SIMD_COMPILER_HAS_REQUIRED_FEATURES
# define SIMD_COMPILER_HAS_REQUIRED_FEATURES 0
#endif

#if SIMD_COMPILER_HAS_REQUIRED_FEATURES

/* thanks C++ */
#ifdef __cplusplus
using namespace simd;
#endif

__attribute__((always_inline))
static inline gvec(float,4) SINvf3(gvec(float,4) x) {
	simd_float3 xvals = {x[0], x[1], x[2]};
	simd_float3 sins = sin(xvals);
	gvec(float,4) ret = {sins[0], sins[1], sins[2]};
	return ret;
}

__attribute__((always_inline))
static inline void SINCOSv2(gvec(double,2) x, gvec(double,2) *s, gvec(double,2)
		*c) {
	simd_double2 xvals = {x[0], x[1]};
	sincos(xvals, (simd_double2 *)s, (simd_double2 *)c);
}

__attribute__((always_inline))
static inline void SINCOSvf4(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	simd_float4 xvals = {x[0], x[1], x[2], x[3]};
	sincos(xvals, (simd_float4 *)s, (simd_float4 *)c);
}
__attribute__((always_inline))
static inline void SINCOSvf3(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	simd_float3 xvals = {x[0], x[1], x[2]};
	sincos(xvals, (simd_float3 *)s, (simd_float3 *)c);
}
__attribute__((always_inline))
static inline void SINCOSvf2(gvec(float,2) x, gvec(float,2) *s, gvec(float,2)
		*c) {
	simd_float2 xvals = {x[0], x[1]};
	sincos(xvals, (simd_float2 *)s, (simd_float2 *)c);
}

__attribute__((always_inline))
static inline void SINCOSPIv2(gvec(double,2) x, gvec(double,2) *s,
		gvec(double,2) *c) {
	simd_double2 xvals = {x[0], x[1]};
	sincospi(xvals, (simd_double2 *)s, (simd_double2 *)c);
}

__attribute__((always_inline))
static inline void SINCOSPIvf4(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	simd_float4 xvals = {x[0], x[1], x[2], x[3]};
	sincospi(xvals, (simd_float4 *)s, (simd_float4 *)c);
}
__attribute__((always_inline))
static inline void SINCOSPIvf3(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	simd_float3 xvals = {x[0], x[1], x[2]};
	sincospi(xvals, (simd_float3 *)s, (simd_float3 *)c);
}
__attribute__((always_inline))
static inline void SINCOSPIvf2(gvec(float,2) x, gvec(float,2) *s, gvec(float,2)
		*c) {
	simd_float2 xvals = {x[0], x[1]};
	sincospi(xvals, (simd_float2 *)s, (simd_float2 *)c);
}
#else
__attribute__((always_inline))
static inline gvec(float,4) SINvf3(gvec(float,4) x) {
	for (int i = 0; i < 3; ++i)
		x[i] = sinf(x[i]);

	return x;
}

__attribute__((always_inline))
static inline void SINCOSv2(gvec(double,2) x, gvec(double,2) *s, gvec(double,2)
		*c) {
	double sins[2], coss[2];
	for (int i = 0; i < 2; ++i)
		SINCOS(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 2; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}

__attribute__((always_inline))
static inline void SINCOSvf4(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	float sins[4], coss[4];
	for (int i = 0; i < 4; ++i)
		SINCOSf(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 4; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}
__attribute__((always_inline))
static inline void SINCOSvf3(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	float sins[3], coss[3];
	for (int i = 0; i < 3; ++i)
		SINCOSf(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 3; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}
__attribute__((always_inline))
static inline void SINCOSvf2(gvec(float,2) x, gvec(float,2) *s, gvec(float,2)
		*c) {
	float sins[2], coss[2];
	for (int i = 0; i < 2; ++i)
		SINCOSf(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 2; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}

__attribute__((always_inline))
static inline void SINCOSPIv2(gvec(double,2) x, gvec(double,2) *s,
		gvec(double,2) *c) {
	double sins[2], coss[2];
	for (int i = 0; i < 2; ++i)
		SINCOSPI(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 2; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}
__attribute__((always_inline))
static inline void SINCOSPIvf4(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	float sins[4], coss[4];
	for (int i = 0; i < 4; ++i)
		SINCOSPIf(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 4; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}
__attribute__((always_inline))
static inline void SINCOSPIvf3(gvec(float,4) x, gvec(float,4) *s, gvec(float,4)
		*c) {
	float sins[3], coss[3];
	for (int i = 0; i < 3; ++i)
		SINCOSPIf(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 3; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}
__attribute__((always_inline))
static inline void SINCOSPIvf2(gvec(float,2) x, gvec(float,2) *s, gvec(float,2)
		*c) {
	float sins[2], coss[2];
	for (int i = 0; i < 2; ++i)
		SINCOSPIf(x[i], &(sins[i]), &(coss[i]));

	for (int i = 0; i < 2; ++i) {
		(*s)[i] = sins[i];
		(*c)[i] = coss[i];
	}
}
#endif /* SIMD_COMPILER_HAS_REQUIRED_FEATURES */

#endif /* MATH_VECTOR_H */
