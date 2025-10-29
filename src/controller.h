#ifndef CONTROLLER_H
#define CONTROLLER_H 1

#include <cppheader.h>
#include <math/vector.h>
#include <pthread.h>

struct control {
	pthread_mutex_t lock;
	gvec(float,2) move;
};

struct control *ctrl_create(struct control *);

void ctrl_destroy(struct control *);

#endif /* CONTROLLER_H */
