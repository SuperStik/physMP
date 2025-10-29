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
