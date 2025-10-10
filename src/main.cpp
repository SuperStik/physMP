/* not having these causes a crash for some reason */
#define JPH_PROFILE_ENABLED 1
#define JPH_DEBUG_RENDERER 1
#define JPH_OBJECT_STREAM 1

#include <cassert>
#include <cstdio>
#include <err.h>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <SDL3/SDL_init.h>

#include "metal/main.h"

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

	const unsigned maxbodies = 65536;
	const unsigned numbodymutexes = 0;
	const unsigned maxbodypairs = 65536;
	const unsigned maxcontactconstraints = 10240;

	IBPLayerImpl bpl_interface;

	ObjVsBPLFilterImpl objbplfilter;

	ObjLayerPairFilterImpl objlayerpairfilter;

	fputs("Hello physMP!\n", stderr);
	MTL_main();

	delete JPH::Factory::sInstance;

	SDL_Quit();
	return 0;
}
