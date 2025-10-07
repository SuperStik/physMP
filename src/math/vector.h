#ifndef VECTOR_H
#define VECTOR_H 1
#define _GNU_SOURCE /* evil in a header file, I know */

#include <math.h>

#ifdef __APPLE__
# include <simd/math.h>
#endif /* __APPLE__ */

#define gvec(type, elems) type __attribute__((vector_size(sizeof(type)*elems)))

#ifndef SIMD_COMPILER_HAS_REQUIRED_FEATURES
# define SIMD_COMPILER_HAS_REQUIRED_FEATURES 0
#endif

#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ == 41))
# define SINPI(x) sinpi(x)
# define COSPI(x) cospi(x)
# define TANPI(x) tanpi(x)

# define SINPIf(x) sinpif(x)
# define COSPIf(x) cospif(x)
# define TANPIf(x) tanpif(x)
#elif defined(__APPLE__)
# define SINPI(x) __sinpi(x)
# define COSPI(x) __cospi(x)
# define TANPI(x) __tanpi(x)

# define SINPIf(x) __sinpif(x)
# define COSPIf(x) __cospif(x)
# define TANPIf(x) __tanpif(x)
#else
# define SINPI(x) sin((x) * M_PI)
# define COSPI(x) cos((x) * M_PI)
# define TANPI(x) tan((x) * M_PI)

# define SINPIf(x) sinf((x) * (float)M_PI);
# define COSPIf(x) cosf((x) * (float)M_PI);
# define TANPIf(x) tanf((x) * (float)M_PI);
#endif

#ifdef __APPLE__
# define SINCOS(x, s, c) __sincos(x, s, c)
# define SINCOSf(x, s, c) __sincosf(x, s, c)
#elif defined(__gnu_linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define SINCOS(x, s, c) sincos(x, s, c)
# define SINCOSf(x, s, c) sincosf(x, s, c)
#else
__attribute__((always_inline))
static inline void SINCOS(double x, double *s, double *c) {
	*s = sin(s);
	*c = cos(s);
}
__attribute__((always_inline))
static inline void SINCOSf(float x, float *s, float *c) {
	*s = sinf(s);
	*c = cosf(s);
}
#endif

#ifdef __APPLE__
# define SINCOSPI(x, s, c) __sincospi(x, s, c)
# define SINCOSPIf(x, s, c) __sincospif(x, s, c)
#else
# define SINCOSPI(x, s, c) SINCOS((x) * M_PI, s, c)
# define SINCOSPIf(x, s, c) SINCOSf((x) * (float)M_PI, s, c)
#endif

#if SIMD_COMPILER_HAS_REQUIRED_FEATURES
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
#endif /* SIMD_COMPILER_HAS_REQUIRED_FEATURES */

#endif /* VECTOR_H */
