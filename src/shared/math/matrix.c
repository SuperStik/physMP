#include <err.h>
#include <stdint.h>
#include <string.h>

#ifdef __SSE3__
# include <pmmintrin.h>
#endif /* __SSE3__ */

#if defined(__AVX2__) || defined(__FMA__)
# include <immintrin.h>
#endif /* __AVX2__ || __FMA__ */

#include "matrix.h"
#include "vector.h"

#ifdef CLANG_MATRIX
typedef float float4x4 __attribute__((matrix_type(4,4)));
#endif

const gvec(float,4) mtx_identity[4] = {
	{1.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 1.0f}
};

gvec(float,4) *mtx_gettranslate(gvec(float,4) mtx[4], float x, float y, float
		z) {
	mtx[0] = (gvec(float,4)){1.0f, 0.0f, 0.0f, 0.0f};

	mtx[1] = (gvec(float,4)){0.0f, 1.0f, 0.0f, 0.0f};

	mtx[2] = (gvec(float,4)){0.0f, 0.0f, 1.0f, 0.0f};

	mtx[3] = (gvec(float,4)){x, y, z, 1.0f};

	return mtx;
}

gvec(float,4) *mtx_getscale(gvec(float,4) mtx[4], float x, float y, float z) {
	mtx[0] = (gvec(float,4)){x, 0.0f, 0.0f, 0.0f};

	mtx[1] = (gvec(float,4)){0.0f, y, 0.0f, 0.0f};

	mtx[2] = (gvec(float,4)){0.0f, 0.0f, z, 0.0f};

	mtx[3] = (gvec(float,4)){0.0f, 0.0f, 0.0f, 1.0f};

	return mtx;
}

gvec(float,4) *mtx_getrotate(gvec(float,4) mtx[4], gvec(float,4) q) {
	gvec(float,4) qsqr = q * q;

	/* these have their transpose pre-computed */
	mtx[0] = (gvec(float,4)){
		qsqr[0] + qsqr[1] - qsqr[2] - qsqr[3],
		2.0f * (q[1] * q[2] + q[0] * q[3]),
		2.0f * (q[1] * q[3] - q[0] * q[2]),
		0.0f
	};

	mtx[1] = (gvec(float,4)) {
		2.0f * (q[1] * q[2] - q[0] * q[3]),
		qsqr[0] - qsqr[1] + qsqr[2] - qsqr[3],
		2.0f * (q[2] * q[3] + q[0] * q[1]),
		0.0f
	};

	mtx[2] = (gvec(float,4)){
		2.0f * (q[1] * q[3] + q[0] * q[2]),
		2.0f * (q[2] * q[3] - q[0] * q[1]),
		qsqr[0] - qsqr[1] - qsqr[2] + qsqr[3],
		0.0f
	};

	mtx[3] = (gvec(float,4)){0.0f, 0.0f, 0.0f, 1.0f};

	return mtx;
}

gvec(float,4) *mtx_translate(gvec(float,4) mtx[4], float x, float y, float z) {
	gvec(float,4) trans[4];
	gvec(float,4) ret[4];

	mtx_gettranslate(trans, x, y, z);
	mtx_mul(trans, mtx, ret);

	memcpy(mtx, ret, sizeof(ret));

	return mtx;
}

gvec(float,4) *mtx_scale(gvec(float,4) mtx[4], float x, float y, float z) {
	gvec(float,4) scale[4];
	gvec(float,4) ret[4];

	mtx_getscale(scale, x, y, z);
	mtx_mul(scale, mtx, ret);

	memcpy(mtx, ret, sizeof(ret));

	return mtx;
}

gvec(float,4) *mtx_rotate(gvec(float,4) mtx[4], gvec(float,4) q) {
	gvec(float,4) rot[4];
	gvec(float,4) ret[4];

	mtx_getrotate(rot, q);
	mtx_mul(rot, mtx, ret);

	memcpy(mtx, ret, sizeof(ret));

	return mtx;
}

gvec(float,4) *mtx_mul(const gvec(float,4) a[4], const gvec(float,4) b[4],
		gvec(float,4) *restrict c) {
#ifdef CLANG_MATRIX
	const float4x4 *mtx_a = (float4x4 *)a;
	const float4x4 *mtx_b = (float4x4 *)b;
	float4x4 *mtx_c = (float4x4 *)c;

	*mtx_c = *mtx_b * *mtx_a;
#else
# ifdef __AVX2__
	const __m128i index = _mm_set_epi32(12, 8, 4, 0);
# endif /* __AVX2__ */

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			gvec(float,4) rowvec = a[i];
			gvec(float,4) colvec;
# ifdef __AVX2__
			union {
				gvec(float,4) vec;
				__m128 mm;
			} swap;

			swap.mm = _mm_i32gather_ps((float *)b + j, index, 4);

			colvec = swap.vec;
# else
			colvec = (gvec(float,4)){b[0][j], b[1][j], b[2][j],
					b[3][j]};
# endif /* __AVX2__ */

			gvec(float,4) dot = rowvec * colvec;
			c[i][j] = (dot[0] + dot[1]) + (dot[2] + dot[3]);
		}
	}
#endif /* CLANG_MATRIX */

	return c;
}

gvec(float,4) *mtx_transpose(const gvec(float,4) a[4], gvec(float,4) b[4]) {
#ifdef CLANG_MATRIX
	*(float4x4 *)b = __builtin_matrix_transpose(*(float4x4 *)a);
#elif defined(__SSE__)
	union {
		gvec(float,4) vec[4];
		__m128 mm[4];
	} temp;
	for (int i = 0; i < 4; ++i)
		temp.vec[i] = a[i];

	_MM_TRANSPOSE4_PS(temp.mm[0], temp.mm[1], temp.mm[2], temp.mm[3]);

	for (int i = 0; i < 4; ++i)
		b[i] = temp.vec[i];
#else
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j)
			b[j][i] = a[i][j];
	}
#endif

	return b;
}

/* this is awful */
gvec(float,4) *mtx_inverse(const gvec(float,4) a[4], gvec(float,4) b[4]) {
	gvec(float,4) inv[4], detvec;
	float det;

	inv[0][0] = a[1][1] * a[2][2] * a[3][3] -
		a[1][1] * a[2][3] * a[3][2] -
		a[2][1] * a[1][2] * a[3][3] +
		a[2][1] * a[1][3] * a[3][2] +
		a[3][1] * a[1][2] * a[2][3] -
		a[3][1] * a[1][3] * a[2][2];

	inv[1][0] = -a[1][0] * a[2][2] * a[3][3] +
		a[1][0] * a[2][3] * a[3][2] +
		a[2][0] * a[1][2] * a[3][3] -
		a[2][0] * a[1][3] * a[3][2] -
		a[3][0] * a[1][2] * a[2][3] +
		a[3][0] * a[1][3] * a[2][2];

	inv[2][0] = a[1][0] * a[2][1] * a[3][3] -
		a[1][0] * a[2][3] * a[3][1] -
		a[2][0] * a[1][1] * a[3][3] +
		a[2][0] * a[1][3] * a[3][1] +
		a[3][0] * a[1][1] * a[2][3] -
		a[3][0] * a[1][3] * a[2][1];

	inv[3][0] = -a[1][0] * a[2][1] * a[3][2] +
		a[1][0] * a[2][2] * a[3][1] +
		a[2][0] * a[1][1] * a[3][2] -
		a[2][0] * a[1][2] * a[3][1] -
		a[3][0] * a[1][1] * a[2][2] +
		a[3][0] * a[1][2] * a[2][1];

	det = (inv[0][0] * a[0][0] + inv[1][0] * a[0][1]) + (inv[2][0] * a[0][2]
			+ inv[3][0] * a[0][3]);
	if (__builtin_expect(det == 0.0f, 0))
		return NULL;

	inv[0][1] = -a[0][1] * a[2][2] * a[3][3] +
		a[0][1] * a[2][3] * a[3][2] +
		a[2][1] * a[0][2] * a[3][3] -
		a[2][1] * a[0][3] * a[3][2] -
		a[3][1] * a[0][2] * a[2][3] +
		a[3][1] * a[0][3] * a[2][2];

	inv[1][1] = a[0][0] * a[2][2] * a[3][3] -
		a[0][0] * a[2][3] * a[3][2] -
		a[2][0] * a[0][2] * a[3][3] +
		a[2][0] * a[0][3] * a[3][2] +
		a[3][0] * a[0][2] * a[2][3] -
		a[3][0] * a[0][3] * a[2][2];

	inv[2][1] = -a[0][0] * a[2][1] * a[3][3] +
		a[0][0] * a[2][3] * a[3][1] +
		a[2][0] * a[0][1] * a[3][3] -
		a[2][0] * a[0][3] * a[3][1] -
		a[3][0] * a[0][1] * a[2][3] +
		a[3][0] * a[0][3] * a[2][1];

	inv[3][1] = a[0][0] * a[2][1] * a[3][2] -
		a[0][0] * a[2][2] * a[3][1] -
		a[2][0] * a[0][1] * a[3][2] +
		a[2][0] * a[0][2] * a[3][1] +
		a[3][0] * a[0][1] * a[2][2] -
		a[3][0] * a[0][2] * a[2][1];

	inv[0][2] = a[0][1] * a[1][2] * a[3][3] -
		a[0][1] * a[1][3] * a[3][2] -
		a[1][1] * a[0][2] * a[3][3] +
		a[1][1] * a[0][3] * a[3][2] +
		a[3][1] * a[0][2] * a[1][3] -
		a[3][1] * a[0][3] * a[1][2];

	inv[1][2] = -a[0][0] * a[1][2] * a[3][3] +
		a[0][0] * a[1][3] * a[3][2] +
		a[1][0] * a[0][2] * a[3][3] -
		a[1][0] * a[0][3] * a[3][2] -
		a[3][0] * a[0][2] * a[1][3] +
		a[3][0] * a[0][3] * a[1][2];

	inv[2][2] = a[0][0] * a[1][1] * a[3][3] -
		a[0][0] * a[1][3] * a[3][1] -
		a[1][0] * a[0][1] * a[3][3] +
		a[1][0] * a[0][3] * a[3][1] +
		a[3][0] * a[0][1] * a[1][3] +
		a[3][0] * a[0][3] * a[1][1];

	inv[3][2] = -a[0][0] * a[1][1] * a[3][2] +
		a[0][0] * a[1][2] * a[3][1] +
		a[1][0] * a[0][1] * a[3][2] -
		a[1][0] * a[0][2] * a[3][1] -
		a[3][0] * a[0][1] * a[1][2] +
		a[3][0] * a[0][2] * a[1][1];

	inv[0][3] = -a[0][1] * a[1][2] * a[2][3] +
		a[0][1] * a[1][3] * a[2][2] +
		a[1][1] * a[0][2] * a[2][3] -
		a[1][1] * a[0][3] * a[2][2] -
		a[2][1] * a[0][2] * a[1][3] +
		a[2][1] * a[0][3] * a[1][2];

	inv[1][3] = a[0][0] * a[1][2] * a[2][3] -
		a[0][0] * a[1][3] * a[2][2] -
		a[1][0] * a[0][2] * a[2][3] +
		a[1][0] * a[0][3] * a[2][2] +
		a[2][0] * a[0][2] * a[1][3] -
		a[2][0] * a[0][3] * a[1][2];

	inv[2][3] = -a[0][0] * a[1][1] * a[2][3] +
		a[0][0] * a[1][3] * a[2][1] +
		a[1][0] * a[0][1] * a[2][3] -
		a[1][0] * a[0][3] * a[2][1] -
		a[2][0] * a[0][1] * a[1][3] +
		a[2][0] * a[0][3] * a[1][1];

	inv[3][3] = a[0][0] * a[1][1] * a[2][2] -
		a[0][0] * a[1][2] * a[2][1] -
		a[1][0] * a[0][1] * a[2][2] +
		a[1][0] * a[0][2] * a[2][1] +
		a[2][0] * a[0][1] * a[1][2] -
		a[2][0] * a[0][2] * a[1][1];

	for (int i = 0; i < 4; ++i)
		b[i] = inv[i] / det;

	return b;
}

gvec(float,4) *mtx_inverse_t(const gvec(float,4) a[4], gvec(float,4) b[4]) {
	gvec(float,4) inv[4], detvec;
	float det;

	inv[0][0] = a[1][1] * a[2][2] * a[3][3] -
		a[1][1] * a[2][3] * a[3][2] -
		a[2][1] * a[1][2] * a[3][3] +
		a[2][1] * a[1][3] * a[3][2] +
		a[3][1] * a[1][2] * a[2][3] -
		a[3][1] * a[1][3] * a[2][2];

	inv[0][1] = -a[1][0] * a[2][2] * a[3][3] +
		a[1][0] * a[2][3] * a[3][2] +
		a[2][0] * a[1][2] * a[3][3] -
		a[2][0] * a[1][3] * a[3][2] -
		a[3][0] * a[1][2] * a[2][3] +
		a[3][0] * a[1][3] * a[2][2];

	inv[0][2] = a[1][0] * a[2][1] * a[3][3] -
		a[1][0] * a[2][3] * a[3][1] -
		a[2][0] * a[1][1] * a[3][3] +
		a[2][0] * a[1][3] * a[3][1] +
		a[3][0] * a[1][1] * a[2][3] -
		a[3][0] * a[1][3] * a[2][1];

	inv[0][3] = -a[1][0] * a[2][1] * a[3][2] +
		a[1][0] * a[2][2] * a[3][1] +
		a[2][0] * a[1][1] * a[3][2] -
		a[2][0] * a[1][2] * a[3][1] -
		a[3][0] * a[1][1] * a[2][2] +
		a[3][0] * a[1][2] * a[2][1];

	detvec = inv[0] * a[0];
	det = (detvec[0] + detvec[1]) + (detvec[2] + detvec[3]);
	if (__builtin_expect(det == 0.0f, 0))
		return NULL;

	inv[1][0] = -a[0][1] * a[2][2] * a[3][3] +
		a[0][1] * a[2][3] * a[3][2] +
		a[2][1] * a[0][2] * a[3][3] -
		a[2][1] * a[0][3] * a[3][2] -
		a[3][1] * a[0][2] * a[2][3] +
		a[3][1] * a[0][3] * a[2][2];

	inv[1][1] = a[0][0] * a[2][2] * a[3][3] -
		a[0][0] * a[2][3] * a[3][2] -
		a[2][0] * a[0][2] * a[3][3] +
		a[2][0] * a[0][3] * a[3][2] +
		a[3][0] * a[0][2] * a[2][3] -
		a[3][0] * a[0][3] * a[2][2];

	inv[1][2] = -a[0][0] * a[2][1] * a[3][3] +
		a[0][0] * a[2][3] * a[3][1] +
		a[2][0] * a[0][1] * a[3][3] -
		a[2][0] * a[0][3] * a[3][1] -
		a[3][0] * a[0][1] * a[2][3] +
		a[3][0] * a[0][3] * a[2][1];

	inv[1][3] = a[0][0] * a[2][1] * a[3][2] -
		a[0][0] * a[2][2] * a[3][1] -
		a[2][0] * a[0][1] * a[3][2] +
		a[2][0] * a[0][2] * a[3][1] +
		a[3][0] * a[0][1] * a[2][2] -
		a[3][0] * a[0][2] * a[2][1];

	inv[2][0] = a[0][1] * a[1][2] * a[3][3] -
		a[0][1] * a[1][3] * a[3][2] -
		a[1][1] * a[0][2] * a[3][3] +
		a[1][1] * a[0][3] * a[3][2] +
		a[3][1] * a[0][2] * a[1][3] -
		a[3][1] * a[0][3] * a[1][2];

	inv[2][1] = -a[0][0] * a[1][2] * a[3][3] +
		a[0][0] * a[1][3] * a[3][2] +
		a[1][0] * a[0][2] * a[3][3] -
		a[1][0] * a[0][3] * a[3][2] -
		a[3][0] * a[0][2] * a[1][3] +
		a[3][0] * a[0][3] * a[1][2];

	inv[2][2] = a[0][0] * a[1][1] * a[3][3] -
		a[0][0] * a[1][3] * a[3][1] -
		a[1][0] * a[0][1] * a[3][3] +
		a[1][0] * a[0][3] * a[3][1] +
		a[3][0] * a[0][1] * a[1][3] +
		a[3][0] * a[0][3] * a[1][1];

	inv[2][3] = -a[0][0] * a[1][1] * a[3][2] +
		a[0][0] * a[1][2] * a[3][1] +
		a[1][0] * a[0][1] * a[3][2] -
		a[1][0] * a[0][2] * a[3][1] -
		a[3][0] * a[0][1] * a[1][2] +
		a[3][0] * a[0][2] * a[1][1];

	inv[3][0] = -a[0][1] * a[1][2] * a[2][3] +
		a[0][1] * a[1][3] * a[2][2] +
		a[1][1] * a[0][2] * a[2][3] -
		a[1][1] * a[0][3] * a[2][2] -
		a[2][1] * a[0][2] * a[1][3] +
		a[2][1] * a[0][3] * a[1][2];

	inv[3][1] = a[0][0] * a[1][2] * a[2][3] -
		a[0][0] * a[1][3] * a[2][2] -
		a[1][0] * a[0][2] * a[2][3] +
		a[1][0] * a[0][3] * a[2][2] +
		a[2][0] * a[0][2] * a[1][3] -
		a[2][0] * a[0][3] * a[1][2];

	inv[3][2] = -a[0][0] * a[1][1] * a[2][3] +
		a[0][0] * a[1][3] * a[2][1] +
		a[1][0] * a[0][1] * a[2][3] -
		a[1][0] * a[0][3] * a[2][1] -
		a[2][0] * a[0][1] * a[1][3] +
		a[2][0] * a[0][3] * a[1][1];

	inv[3][3] = a[0][0] * a[1][1] * a[2][2] -
		a[0][0] * a[1][2] * a[2][1] -
		a[1][0] * a[0][1] * a[2][2] +
		a[1][0] * a[0][2] * a[2][1] +
		a[2][0] * a[0][1] * a[1][2] -
		a[2][0] * a[0][2] * a[1][1];

	for (int i = 0; i < 4; ++i)
		b[i] = inv[i] / det;

	return b;
}

gvec(float,4) *mtx_add(const gvec(float,4) a[4], const gvec(float,4) b[4],
		gvec(float,4) *restrict c) {
#ifdef CLANG_MATRIX
	const float4x4 *mtx_a = (float4x4 *)a;
	const float4x4 *mtx_b = (float4x4 *)b;

	*(float4x4 *)c = *mtx_a + *mtx_b;
#else
	for (int i = 0; i < 4; ++i)
		c[i] = a[i] + b[i];
#endif /* CLANG_MATRIX */

	return c;
}

gvec(float,4) *mtx_smul(const gvec(float,4) a[4], float s, gvec(float,4) *
		restrict b) {
#ifdef CLANG_MATRIX
	const float4x4 *mtx_a = (float4x4 *)a;

	*(float4x4 *)b = *mtx_a * s;
#else
	for (int i = 0; i < 4; ++i)
		b[i] = s * a[i];
#endif /* CLANG_MATRIX */

	return b;
}
