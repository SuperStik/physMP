#ifndef ANGLE_H
#define ANGLE_H 1

#include <math/vector.h>

gvec(float,4) ang_eul2quat(float p, float y, float r);
gvec(float,4) ang_eulnoroll2quat(float p, float y);

gvec(float,4) ang_axisang2quat(gvec(float,4));

gvec(float,4) ang_quat2axisang(gvec(float,4));

#endif /* ANGLE_H */
