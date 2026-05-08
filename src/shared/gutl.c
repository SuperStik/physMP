#include <err.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "gutl.h"
#include "math/vector.h"

float *GUTL_perspectivef(float matrix[16], float fovy, float aspect, float
		zNear, float zFar) {
	float tanhalffov = TANPIf(fovy / 360.0f);
	float zRange = zFar / (zNear - zFar);

	gvec(float,2) cothalffov = {tanhalffov * aspect, tanhalffov};
	cothalffov = 1.0f / cothalffov;

	matrix[0] = cothalffov[0];
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = cothalffov[1];
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = -zRange;
	matrix[11] = 1.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = zNear * zRange;
	matrix[15] = 0.0f;

	return matrix;
}

double *GUTL_perspectived(double matrix[16], double fovy, double aspect, double
		zNear, double zFar) {
	double tanhalffov = TANPI(fovy / 360.0f);
	double zRange = zFar / (zNear - zFar);

	gvec(double,2) cothalffov = {tanhalffov * aspect, tanhalffov};
	cothalffov = 1.0f / cothalffov;

	matrix[0] = cothalffov[0];
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	matrix[3] = 0.0;

	matrix[4] = 0.0;
	matrix[5] = cothalffov[1];
	matrix[6] = 0.0;
	matrix[7] = 0.0;

	matrix[8] = 0.0;
	matrix[9] = 0.0;
	matrix[10] = -zRange;
	matrix[11] = 1.0;

	matrix[12] = 0.0;
	matrix[13] = 0.0;
	matrix[14] = zNear * zRange;
	matrix[15] = 0.0;

	return matrix;
}

float *GUTL_orthof(float matrix[16], float left, float right, float bottom,
		float top, float zNear, float zFar) {
	gvec(float,4) vals = {right - left, top - bottom, zFar - zNear};
	vals = 1.0f / vals;

	gvec(float,4) valsm2 = {2.0f, 2.0f, 1.0f};
	valsm2 *= vals;

	matrix[0] = valsm2[0];
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;
	matrix[4] = 0.0f;
	matrix[5] = valsm2[1];
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;
	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = valsm2[2];
	matrix[11] = 0.0f;
	matrix[12] = -(right + left) * vals[0];
	matrix[13] = -(top + bottom) * vals[1];
	matrix[14] = -zNear * vals[2];
	matrix[15] = 1.0f;

	return matrix;
}

double *GUTL_orthod(double matrix[16], double left, double right, double bottom,
		double top, double zNear, double zFar) {
	gvec(double,4) vals = {right - left, top - bottom, zFar - zNear};
	vals = 1.0 / vals;

	gvec(double,4) valsm2 = {2.0, 2.0, -2.0};
	valsm2 *= vals;

	matrix[0] = valsm2[0];
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	matrix[3] = 0.0;
	matrix[4] = 0.0;
	matrix[5] = valsm2[1];
	matrix[6] = 0.0;
	matrix[7] = 0.0;
	matrix[8] = 0.0;
	matrix[9] = 0.0;
	matrix[10] = valsm2[2];
	matrix[11] = 0.0;
	matrix[12] = -(right + left) * vals[0];
	matrix[13] = -(top + bottom) * vals[1];
	matrix[14] = -(zFar + zNear) * vals[2];
	matrix[15] = 1.0;

	return matrix;
}
