#include <clocale>
#include <cstdio>
#include <err.h>

#include <SDL3/SDL_init.h>

#include "metal/main.h"
#include "phys.h"

C_BEGIN;
char done = 0;
C_END;

int main(void) {
	setlocale(LC_ALL, "");

	char initialized = SDL_Init(SDL_INIT_VIDEO);
	if (!initialized)
		errx(1, "%s", SDL_GetError());

	pthread_t physthr = phys_begin();

	fputs("Hello physMP!\n", stderr);
	MTL_main();

	phys_end(physthr);

	SDL_Quit();
	return 0;
}
