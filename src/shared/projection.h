#ifndef PROJECTION_H
#define PROJECTION_H 1

float *proj_perspective(float matrix[16], float fovy, float aspect, float zNear,
		float zFar);

float *proj_ortho(float matrix[16], float left, float right, float bottom, float
		top, float zNear, float zFar);

#endif /* PROJECTION_H */
