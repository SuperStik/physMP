#include <err.h>

#include "events.h"

void ev_key_down(SDL_Window *window, SDL_KeyboardEvent *key) {
	warnx("keydown %u", key->scancode);
}

void ev_key_up(SDL_Window *window, SDL_KeyboardEvent *key) {
	warnx("keyup %u", key->scancode);
}
