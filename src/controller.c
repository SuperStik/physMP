#include <err.h>
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

	gvec(float,2) move = ctrl->move_nonorm;

	switch(key->scancode) {
		case SDL_SCANCODE_W:
			++move[1];
			break;
		case SDL_SCANCODE_S:
			--move[1];
			break;
		case SDL_SCANCODE_A:
			--move[0];
			break;
		case SDL_SCANCODE_D:
			++move[0];
			break;
		default:
			goto donothing;
	}

	ctrl->move_nonorm = move;
	gvec(float,2) movesqr = move * move;

	float lensqr = movesqr[0] + movesqr[1];
	if (lensqr > 1.0f)
		move /= sqrtf(lensqr);

	ctrl->move = move;

	warnx("(%g,%g)", move[0], move[1]);

donothing:
	pthread_mutex_unlock(&(ctrl->lock));
}

void ctrl_keyup(struct control *ctrl, SDL_KeyboardEvent *key) {
	pthread_mutex_lock(&(ctrl->lock));

	gvec(float,2) move = ctrl->move_nonorm;

	switch(key->scancode) {
		case SDL_SCANCODE_W:
			--move[1];
			break;
		case SDL_SCANCODE_S:
			++move[1];
			break;
		case SDL_SCANCODE_A:
			++move[0];
			break;
		case SDL_SCANCODE_D:
			--move[0];
			break;
		default:
			goto donothing;
	}

	ctrl->move_nonorm = move;
	gvec(float,2) movesqr = move * move;

	float lensqr = movesqr[0] + movesqr[1];
	if (lensqr > 1.0f)
		move /= sqrtf(lensqr);

	ctrl->move = move;

	warnx("(%g,%g)", move[0], move[1]);

donothing:
	pthread_mutex_unlock(&(ctrl->lock));
}
