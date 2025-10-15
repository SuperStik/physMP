#ifndef PLAYER_H
#define PLAYER_H 1

#include <cppheader.h>
#include <math/vector.h>

C_BEGIN;

struct player {
	gvec(float,2) eyeangles;
	gvec(float,2) move;
	void *vchar;
	gvec(float,4) *transform;
};

struct player *player_create(struct player *, void *physics_system);

void player_destroy(struct player *);

void player_turn(struct player *, float dx, float dy);

void player_gettransform(struct player *, gvec(float,4) matrix[4]);

void player_physupdate(struct player *, float delta, const void *physics_system,
		const void *extended_update_settings, void *tempalloc);

C_END;

#endif /* PLAYER_H */
