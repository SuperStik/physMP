#ifndef EVENTS_H
#define EVENTS_H 1

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <cppheader.h>

C_BEGIN;

void ev_key_down(SDL_Window *, SDL_KeyboardEvent *);

void ev_key_up(SDL_Window *, SDL_KeyboardEvent *);

C_END;

#endif /* EVENTS_H */
