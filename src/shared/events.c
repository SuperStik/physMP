#include <err.h>

#include "controller.h"
#include "events.h"
#include "shared.h"

void ev_key_down(SDL_Window *window, SDL_KeyboardEvent *key) {
	if (key->repeat)
		return;

	ctrl_keydown(&localplayer.controller, key);
}

void ev_key_up(SDL_Window *window, SDL_KeyboardEvent *key) {
	ctrl_keyup(&localplayer.controller, key);
}
