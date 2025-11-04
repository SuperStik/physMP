#ifndef CONTROLLER_H
#define CONTROLLER_H 1

#include <SDL3/SDL_events.h>

#include <cppheader.h>
#include <math/vector.h>
#include <pthread.h>

#define CONTROL_INITIALIZER {\
	PTHREAD_MUTEX_INITIALIZER, \
	{0.0f, 0.0f} \
}

C_BEGIN;

struct control {
	pthread_mutex_t lock;
	gvec(float,2) move;
};

struct control *ctrl_create(struct control *);

void ctrl_destroy(struct control *);

C_END;

#endif /* CONTROLLER_H */
