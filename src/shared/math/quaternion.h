#ifndef ANGLE_H
#define ANGLE_H 1

#include <cppheader.h>
#include <math/vector.h>

C_BEGIN;
gvec(float,4) quat_from_eul(float p, float y, float r);
gvec(float,4) quat_from_eulnoroll(float p, float y);

gvec(float,4) quat_to_axisang(gvec(float,4));

gvec(float,4) quat_mul(gvec(float,4));
C_END;

#endif /* ANGLE_H */
