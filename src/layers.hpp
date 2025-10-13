#ifndef LAYERS_HPP
#define LAYERS_HPP 1

#include <cassert>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/PhysicsSystem.h>

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

#endif /* LAYERS_HPP */
