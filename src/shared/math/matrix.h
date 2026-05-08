#ifndef MATRIX_H
#define MATRIX_H 1

#include <cppheader.h>
#include <math/vector.h>

C_BEGIN;

extern const gvec(float,4) mtx_identity[4];
#define MTX_IDENTITY_INITIALIZER {\
	{1.0f, 0.0f, 0.0f, 0.0f},\
	{0.0f, 1.0f, 0.0f, 0.0f},\
	{0.0f, 0.0f, 1.0f, 0.0f},\
	{0.0f, 0.0f, 0.0f, 1.0f},\
}

gvec(float,4) *mtx_gettranslate(gvec(float,4) mtx[4], float x, float y, float
		z);

gvec(float,4) *mtx_getscale(gvec(float,4) mtx[4], float x, float y, float z);

gvec(float,4) *mtx_getrotate(gvec(float,4) mtx[4], gvec(float,4) q);

gvec(float,4) *mtx_translate(gvec(float,4) mtx[4], float x, float y, float z);

gvec(float,4) *mtx_scale(gvec(float,4) mtx[4], float x, float y, float z);

gvec(float,4) *mtx_rotate(gvec(float,4) mtx[4], gvec(float,4) q);

gvec(float,4) *mtx_transpose(const gvec(float,4) a[4], gvec(float,4) b[4]);

gvec(float,4) *mtx_inverse(const gvec(float,4) a[4], gvec(float,4) b[4]);
gvec(float,4) *mtx_inverse_t(const gvec(float,4) a[4], gvec(float,4) b[4]);

gvec(float,4) *mtx_mul(const gvec(float,4) a[4], const gvec(float,4) b[4],
		gvec(float,4) *restrict c);

gvec(float,4) *mtx_add(const gvec(float,4) a[4], const gvec(float,4) b[4],
		gvec(float,4) *restrict c);

gvec(float,4) *mtx_smul(const gvec(float,4) a[4], float s, gvec(float,4) *
		restrict b);

C_END;

#endif /* MATRIX_H */
