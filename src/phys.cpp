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
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <SDL3/SDL_timer.h>

#include "math/matrix.h"
#include "phys.h"
#include "shared.h"

C_BEGIN;
extern char done;
gvec(float,4) modelobj[4];
C_END;

using namespace JPH::literals;

namespace BPL {
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr unsigned NUM_LAYERS = 2;
};

namespace Layers {
	enum Layers {
		NON_MOVING,
		MOVING,
		NUM_LAYERS
	};
};

class IBPLayerImpl final : public JPH::BroadPhaseLayerInterface {
	public:
		IBPLayerImpl() {
			tobroadphase[Layers::NON_MOVING] = BPL::NON_MOVING;
			tobroadphase[Layers::MOVING] = BPL::MOVING;
		}

		virtual unsigned GetNumBroadPhaseLayers() const override {
			return BPL::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer
				inLayer) const override {
			assert(inLayer < Layers::NUM_LAYERS);
			return tobroadphase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer
				inLayer) const override {
			switch ((JPH::BroadPhaseLayer::Type)inLayer) {
				case (JPH::BroadPhaseLayer::Type)
					BPL::NON_MOVING:
					return "NON_MOVING";
				case (JPH::BroadPhaseLayer::Type)BPL::MOVING:
					return "MOVING";
				default:
					return "INVALID";
			}
		}
#endif /* JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED */
	private:
		JPH::BroadPhaseLayer tobroadphase[Layers::NUM_LAYERS];
};

class ObjVsBPLFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1,
				JPH::BroadPhaseLayer inLayer2) {
			switch (inLayer1) {
				case Layers::NON_MOVING:
					return inLayer2 == BPL::MOVING;
				case Layers::MOVING:
					return true;
				default:
					return false;
			}
		}
};

class ObjLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) {
			switch (inObject1) {
				case Layers::NON_MOVING:
					return inObject2 == Layers::MOVING;
				case Layers::MOVING:
					return true;
				default:
					return false;
			}
		}
};

static void *simulate(void *physics_system);

static IBPLayerImpl ibpl;

static ObjVsBPLFilterImpl objbplfilter;

static ObjLayerPairFilterImpl objlayerpairfilter;

pthread_t phys_begin() {
	JPH::RegisterDefaultAllocator();

	JPH::Trace = warnx;

	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	const unsigned maxbodies = 65536;
	const unsigned numbodymutexes = 0;
	const unsigned maxbodypairs = 65536;
	const unsigned maxcontactconstraints = 10240;

	JPH::PhysicsSystem *physsys = new JPH::PhysicsSystem();
	physsys->Init(maxbodies, numbodymutexes, maxbodypairs,
			maxcontactconstraints, ibpl, objbplfilter,
			objlayerpairfilter);

	pthread_t thread;
	pthread_create(&thread, NULL, simulate, physsys);
	return thread;
}

void phys_end(pthread_t thread) {
	pthread_join(thread, NULL);

	JPH::UnregisterTypes();

	delete JPH::Factory::sInstance;
}

static void *simulate(void *p) {
	JPH::PhysicsSystem *physsys = static_cast<JPH::PhysicsSystem *>(p);

	JPH::BodyInterface &ibody = physsys->GetBodyInterfaceNoLock();

	JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(256.0f, 1.0f,
				256.0f));
	floor_shape_settings.SetEmbedded();

	JPH::ShapeSettings::ShapeResult floor_shape_result =
		floor_shape_settings.Create();
	JPH::ShapeRefC floor_shape = floor_shape_result.Get();

	JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r,
				-4.5_r, 0.0_r), JPH::Quat::sIdentity(),
				JPH::EMotionType::Static, Layers::NON_MOVING);

	JPH::Body *floor = ibody.CreateBody(floor_settings);
	ibody.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

	JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f),
			JPH::RVec3(0.0_r, 2.0_r, 0.0_r), JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic, Layers::MOVING);
	JPH::BodyID sphere_id = ibody.CreateAndAddBody(sphere_settings,
			JPH::EActivation::Activate);
	ibody.SetLinearVelocity(sphere_id, JPH::Vec3(0.25f, 0.0f, 0.5f));

	physsys->OptimizeBroadPhase();

	JPH::TempAllocatorImpl tempalloc(10 * 1024 * 1024);

	JPH::JobSystemThreadPool jobsys(JPH::cMaxPhysicsJobs,
			JPH::cMaxPhysicsBarriers,
			JPH::thread::hardware_concurrency() - 1);

	uint64_t cl_start;
	unsigned step = 0;

#ifdef __APPLE__
	pthread_setname_np("physMP.physics-thread");
#endif /* __APPLE__ */
	while (ibody.IsActive(sphere_id) && !done) {
		++step;

		cl_start = SDL_GetTicksNS();

		JPH::RVec3 pos = ibody.GetCenterOfMassPosition(sphere_id);
		JPH::Vec3 vel = ibody.GetLinearVelocity(sphere_id);

		const JPH::Mat44 trans = ibody.GetWorldTransform(sphere_id);
		memcpy(modelobj, &trans, sizeof(float) * 16);

		physsys->Update(1.0f / 60.0f, 1, &tempalloc, &jobsys);

		uint64_t duration = SDL_GetTicksNS() - cl_start;
		const uint64_t idealsleeptime = 16666666;
		int64_t sleeptime = idealsleeptime - duration;
		if (sleeptime < 0)
			sleeptime = 0;

		struct timespec ticksleep = {.tv_sec = 0, .tv_nsec = sleeptime};

		nanosleep(&ticksleep, NULL);
	}

	delete physsys;

	return NULL;
}
