#include <err.h>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "player.h"

struct player *player_create(struct player *ply, void *p) {
	auto physsys = static_cast<JPH::PhysicsSystem *>(p);
	JPH::CharacterVirtualSettings plysettings;
	plysettings.mShape = new JPH::CapsuleShape();

	auto vchar = new JPH::CharacterVirtual(&plysettings,
			JPH::RVec3::sZero(), JPH::Quat::sIdentity(), physsys);

	ply->vchar = static_cast<void *>(vchar);
	ply->eyeangles = {0.0f, 0.0f};
	return ply;
}

void player_destroy(struct player *ply) {
	delete static_cast<JPH::CharacterVirtual *>(ply->vchar);
}

/* the things we do for C/C++ interop */
void player_physupdate(struct player *ply, float delta, float gravx, float
		gravy, float gravz, const void *bpl, const void *ol, const
		void *b, const void *s, void *t) {
	auto bplfilter = static_cast<const JPH::BroadPhaseLayerFilter *>(bpl);
	auto olfilter = static_cast<const JPH::ObjectLayerFilter *>(ol);
	auto bfilter = static_cast<const JPH::BodyFilter *>(b);
	auto sfilter = static_cast<const JPH::ShapeFilter *>(s);
	auto tempalloc = static_cast<JPH::TempAllocator *>(t);

	auto vchar = static_cast<JPH::CharacterVirtual *>(ply->vchar);
	vchar->Update(delta, JPH::Vec3(gravx, gravy, gravz), *bplfilter,
			*olfilter, *bfilter, *sfilter, *tempalloc);
	JPH::RVec3 pos = vchar->GetPosition();
	warnx("ply pos: %g %g %g", pos.GetX(), pos.GetY(), pos.GetZ());
}
