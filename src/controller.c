#include <err.h>
#include <stddef.h>

#include "controller.h"

static gvec(float,2) scan2move(SDL_Scancode);

static void movectrl(struct control *, gvec(float,2) move);

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

gvec(float,2) ctrl_getmove(struct control *ctrl) {
	pthread_mutex_lock(&(ctrl->lock));
	gvec(float,2) move = ctrl->move;
	pthread_mutex_unlock(&(ctrl->lock));

	return move;
}

void ctrl_keydown(struct control *ctrl, SDL_KeyboardEvent *key) {
	gvec(float,2) move = scan2move(key->scancode);
	if (move[0] == 0.0f && move[1] == 0.0f)
		return;

	movectrl(ctrl, move);
}

void ctrl_keyup(struct control *ctrl, SDL_KeyboardEvent *key) {
	gvec(float,2) move = scan2move(key->scancode);
	if (move[0] == 0.0f && move[1] == 0.0f)
		return;

	move = -move;

	movectrl(ctrl, move);
}

static gvec(float,2) scan2move(SDL_Scancode scan) {
	gvec(float,2) move = {0.0f, 0.0f};
	switch(scan) {
		case SDL_SCANCODE_W:
			move[1] = 1.0f;
			break;
		case SDL_SCANCODE_S:
			move[1] = -1.0f;
			break;
		case SDL_SCANCODE_A:
			move[0] = -1.0f;
			break;
		case SDL_SCANCODE_D:
			move[0] = 1.0f;
			break;
		default:
			break;
	}

	return move;
}

static void movectrl(struct control *ctrl, gvec(float,2) move) {
	pthread_mutex_lock(&(ctrl->lock));

	move += ctrl->move_nonorm;
	ctrl->move_nonorm = move;
	gvec(float,2) movesqr = move * move;

	float lensqr = movesqr[0] + movesqr[1];
	if (lensqr > 1.0f)
		move /= sqrtf(lensqr);

	ctrl->move = move;

	pthread_mutex_unlock(&(ctrl->lock));
}
