/* not having these causes a crash for some reason */
#define JPH_PROFILE_ENABLED 1
#define JPH_DEBUG_RENDERER 1
#define JPH_OBJECT_STREAM 1

#include <cassert>
#include <cstdint>
#include <ctime>
#include <err.h>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>

#include <SDL3/SDL_timer.h>

#include "layers.hpp"
#include "math/matrix.h"
#include "phys.h"
#include "shared.h"

C_BEGIN;
extern char done;
gvec(float,4) modelobj[4];
struct player localplayer;
C_END;

using namespace JPH;
using namespace literals;

static void *simulate(void *physics_system);

static IBPLayerImpl ibpl;

static ObjVsBPLFilterImpl objbplfilter;

static ObjLayerPairFilterImpl objlayerpairfilter;

pthread_t phys_begin() {
	RegisterDefaultAllocator();

	Trace = warnx;

	Factory::sInstance = new Factory();

	RegisterTypes();

	const unsigned maxbodies = 65536;
	const unsigned numbodymutexes = 0;
	const unsigned maxbodypairs = 65536;
	const unsigned maxcontactconstraints = 10240;

	PhysicsSystem *physsys = new PhysicsSystem();
	physsys->Init(maxbodies, numbodymutexes, maxbodypairs,
			maxcontactconstraints, ibpl, objbplfilter,
			objlayerpairfilter);

	pthread_t thread;
	pthread_create(&thread, NULL, simulate, physsys);
	return thread;
}

void phys_end(pthread_t thread) {
	pthread_join(thread, NULL);

	UnregisterTypes();

	delete Factory::sInstance;
}

static void *simulate(void *p) {
	PhysicsSystem *physsys = static_cast<PhysicsSystem *>(p);

	BodyInterface &ibody = physsys->GetBodyInterfaceNoLock();

	BoxShapeSettings floor_shape_settings(Vec3(128.0f, 1.0f, 128.0f));
	floor_shape_settings.SetEmbedded();

	ShapeSettings::ShapeResult floor_shape_result =
		floor_shape_settings.Create();
	ShapeRefC floor_shape = floor_shape_result.Get();

	BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -4.5_r,
				0.0_r), Quat::sIdentity(), EMotionType::Static,
			Layers::NON_MOVING);

	Body *floor = ibody.CreateBody(floor_settings);
	ibody.AddBody(floor->GetID(), EActivation::DontActivate);

	BodyCreationSettings sphere_settings(new SphereShape(0.25f),
			RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(),
			EMotionType::Dynamic, Layers::MOVING);
	BodyID sphere_id = ibody.CreateAndAddBody(sphere_settings,
			EActivation::Activate);
	ibody.SetLinearVelocity(sphere_id, Vec3(0.25f, 0.0f, 0.5f));

	TempAllocatorImpl tempalloc(10 * 1024 * 1024);

	JobSystemThreadPool jobsys(cMaxPhysicsJobs, cMaxPhysicsBarriers,
			thread::hardware_concurrency() - 1);

	CharacterVirtual::ExtendedUpdateSettings updatesettings;

	struct player *ply = player_create(&localplayer, physsys);

	physsys->OptimizeBroadPhase();

	uint64_t cl_start;

#ifdef __APPLE__
	pthread_setname_np("physMP.physics-thread");
#endif /* __APPLE__ */

	while (!done) {
		cl_start = SDL_GetTicksNS();

		const Mat44 trans = ibody.GetWorldTransform(sphere_id);
		memcpy(modelobj, &trans, sizeof(float) * 16);

		const float delta = 1.0f / 60.0f;
		physsys->Update(delta, 1, &tempalloc, &jobsys);
		player_physupdate(ply, delta, physsys, &updatesettings,
				&tempalloc);

		uint64_t duration = SDL_GetTicksNS() - cl_start;
		const uint64_t idealsleeptime = 16666666;
		int64_t sleeptime = idealsleeptime - duration;
		if (sleeptime < 0)
			sleeptime = 0;

		struct timespec ticksleep = {.tv_sec = 0, .tv_nsec = sleeptime};

		nanosleep(&ticksleep, NULL);
	}

	player_destroy(ply);
	delete physsys;

	return NULL;
}
