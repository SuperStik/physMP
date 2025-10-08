#ifndef PLAYER_H
#define PLAYER_H 1

#include <cppheader.h>

C_BEGIN;

struct player {
	void *physbody;
	float pitch;
	float yaw;
};

C_END;

#endif /* PLAYER_H */
