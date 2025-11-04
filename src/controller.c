#include <stddef.h>

#include "controller.h"

struct control *ctrl_create(struct control *ctrl) {
	if (__builtin_expect(ctrl == NULL, 0))
		return NULL;

	ctrl->move = (gvec(float,2)){0.0f, 0.0f};
	if (__builtin_expect(pthread_mutex_init(&(ctrl->lock), NULL), 0))
		ctrl = NULL;

	return ctrl;
}

void ctrl_destroy(struct control *ctrl) {
	if (__builtin_expect(ctrl == NULL, 0))
		return;

	pthread_mutex_destroy(&(ctrl->lock));
}

void ctrl_keydown(struct control *ctrl, SDL_KeyboardEvent *key) {
	pthread_mutex_lock(&(ctrl->lock));
	pthread_mutex_unlock(&(ctrl->lock));
}

void ctrl_keyup(struct control *ctrl, SDL_KeyboardEvent *key) {
	pthread_mutex_lock(&(ctrl->lock));
	pthread_mutex_unlock(&(ctrl->lock));
}
