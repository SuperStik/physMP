#ifndef PLAYER_H
#define PLAYER_H 1

#include <cppheader.h>
#include <math/vector.h>

C_BEGIN;

struct player {
	gvec(float,2) eyeangles;
	void *vchar;
};

struct player *player_create(struct player *, void *physics_system);

void player_destroy(struct player *);

void player_physupdate(struct player *, float delta, float gravx, float gravy,
		float gravz, const void *bplfilter, const void *olfilter, const
		void *bodyfilter, const void *shapefilter, void *tempalloc);

C_END;

#endif /* PLAYER_H */
