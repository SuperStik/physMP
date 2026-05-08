#ifndef MATH_MISC_H
#define MATH_MISC_H 1

#define _GNU_SOURCE /* evil in a header file, I know */

#include <math.h>

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
	*s = sin(*s);
	*c = cos(*s);
}
__attribute__((always_inline))
static inline void SINCOSf(float x, float *s, float *c) {
	*s = sinf(*s);
	*c = cosf(*s);
}
#endif

#ifdef __APPLE__
# define SINCOSPI(x, s, c) __sincospi(x, s, c)
# define SINCOSPIf(x, s, c) __sincospif(x, s, c)
#else
# define SINCOSPI(x, s, c) SINCOS((x) * M_PI, s, c)
# define SINCOSPIf(x, s, c) SINCOSf((x) * (float)M_PI, s, c)
#endif

#endif /* MATH_MISC_H */
