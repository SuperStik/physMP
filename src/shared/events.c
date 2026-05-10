#include <err.h>

#include "controller.h"
#include "events.h"
#include "shared.h"

void ev_key_down(const SDL_KeyboardEvent *key) {
	if (key->repeat)
		return;

	ctrl_keydown(&localplayer.controller, key->scancode);
}

void ev_key_up(const SDL_KeyboardEvent *key) {
	ctrl_keyup(&localplayer.controller, key->scancode);
}

void ev_mouse_motion(const SDL_MouseMotionEvent *motion) {
	player_turn(&localplayer, motion->xrel, motion->yrel);
}
