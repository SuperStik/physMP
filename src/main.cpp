/* not having these causes a crash for some reason */
#define JPH_PROFILE_ENABLED 1
#define JPH_DEBUG_RENDERER 1
#define JPH_OBJECT_STREAM 1

#include <cstdio>
#include <err.h>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>

#include <SDL3/SDL_init.h>

#include "metal/main.h"

int main(void) {
	char initialized = SDL_Init(SDL_INIT_VIDEO);
	if (!initialized)
		errx(1, "%s", SDL_GetError());

	JPH::RegisterDefaultAllocator();

	JPH::Trace = warnx;

	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	JPH::TempAllocatorImpl(10 * 1024 * 1024);

	JPH::JobSystemThreadPool jobsys(JPH::cMaxPhysicsJobs,
			JPH::cMaxPhysicsBarriers,
			JPH::thread::hardware_concurrency() - 1);

	fputs("Hello physMP!\n", stderr);
	MTL_main();

	delete JPH::Factory::sInstance;

	SDL_Quit();
	return 0;
}
