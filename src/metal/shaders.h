#ifndef SHADERS_H
#define SHADERS_H 1

#include <objc/objc.h>

struct shaders {
	id level;
	id object;
};

struct object_vertdata {
	float pos[3];
	float normal[3];
};

struct shaders *shdr_load(struct shaders *, id device);

void shdr_release(struct shaders *);

#endif /* SHADERS_H */
