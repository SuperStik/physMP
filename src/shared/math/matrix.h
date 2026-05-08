#ifndef MATRIX_H
#define MATRIX_H 1

#include <cppheader.h>
#include <math/vector.h>

C_BEGIN;

extern const gvec(float,4) mat_identity[4];
#define MAT_IDENTITY_INITIALIZER {\
	{1.0f, 0.0f, 0.0f, 0.0f},\
	{0.0f, 1.0f, 0.0f, 0.0f},\
	{0.0f, 0.0f, 1.0f, 0.0f},\
	{0.0f, 0.0f, 0.0f, 1.0f},\
}

gvec(float,4) *mat_gettranslate(gvec(float,4) mat[4], float x, float y, float
		z);

gvec(float,4) *mat_getscale(gvec(float,4) mat[4], float x, float y, float z);

gvec(float,4) *mat_getrotate(gvec(float,4) mat[4], gvec(float,4) q);

gvec(float,4) *mat_translate(gvec(float,4) mat[4], float x, float y, float z);

gvec(float,4) *mat_scale(gvec(float,4) mat[4], float x, float y, float z);

gvec(float,4) *mat_rotate(gvec(float,4) mat[4], gvec(float,4) q);

gvec(float,4) *mat_transpose(const gvec(float,4) a[4], gvec(float,4) b[4]);

gvec(float,4) *mat_inverse(const gvec(float,4) a[4], gvec(float,4) b[4]);
gvec(float,4) *mat_inverse_t(const gvec(float,4) a[4], gvec(float,4) b[4]);

gvec(float,4) *mat_mul(const gvec(float,4) a[4], const gvec(float,4) b[4],
		gvec(float,4) *restrict c);

gvec(float,4) *mat_add(const gvec(float,4) a[4], const gvec(float,4) b[4],
		gvec(float,4) *restrict c);

gvec(float,4) *mat_smul(const gvec(float,4) a[4], float s, gvec(float,4) *
		restrict b);

float *mat_trunc3(const gvec(float,4) a[4], float b[9]);

C_END;

#endif /* MATRIX_H */
