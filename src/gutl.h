#ifndef GUTL_H
#define GUTL_H 1

float *GUTL_perspectivef(float matrix[16], float fovy, float aspect, float
		zNear, float zFar);

double *GUTL_perspectived(double matrix[16], double fovy, double aspect,
		double zNear, double zFar);

float *GUTL_orthof(float matrix[16], float left, float right, float bottom,
		float top, float zNear, float zFar);

double *GUTL_orthod(double matrix[16], double left, double right, double bottom,
		double top, double zNear, double zFar);

#endif /* GUTL_H */
