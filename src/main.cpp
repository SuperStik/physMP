#include <cstdio>
#include <err.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Color.h>
#include <SDL3/SDL_init.h>

#include "metal/main.h"

int main(void) {
	char initialized = SDL_Init(SDL_INIT_VIDEO);
	if (!initialized)
		errx(1, "%s", SDL_GetError());

	const JPH::Color darkred = JPH::Color::sDarkRed;
	printf("darkred: %hhu %hhu %hhu\n", darkred(0), darkred(1), darkred(2));

	fputs("Hello physMP!\n", stderr);
	MTL_main();

	SDL_Quit();
	return 0;
}
