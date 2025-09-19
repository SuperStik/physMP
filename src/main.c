#include <err.h>
#include <stdio.h>

#include <SDL3/SDL_init.h>

#include "metal/main.h"

int main(void) {
	char initialized = SDL_Init(SDL_INIT_VIDEO);
	if (!initialized)
		errx(1, "%s", SDL_GetError());

	fputs("Hello physMP!\n", stderr);
	MTL_main();

	SDL_Quit();
	return 0;
}
