#include <err.h>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "layers.hpp"
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
void player_physupdate(struct player *ply, float delta, const void *s, const void *u, void *t) {
	auto physsys = static_cast<const JPH::PhysicsSystem *>(s);
	auto tempalloc = static_cast<JPH::TempAllocator *>(t);

	auto vchar = static_cast<JPH::CharacterVirtual *>(ply->vchar);

	vchar->UpdateGroundVelocity();

	auto velocity = vchar->GetLinearVelocity();
	auto gravity = physsys->GetGravity();

	auto ground = vchar->GetGroundVelocity();
	JPH::Vec3 new_velocity;
	if (vchar->GetGroundState() ==
			JPH::CharacterVirtual::EGroundState::OnGround)
		new_velocity = ground;
	else
		new_velocity = velocity;
	new_velocity += gravity * delta;

	vchar->SetLinearVelocity(new_velocity);

	auto bplfilter = physsys->GetDefaultBroadPhaseLayerFilter(
			Layers::MOVING);
	auto layfilter = physsys->GetDefaultLayerFilter(Layers::MOVING);
	auto updatesettings = static_cast<const
		JPH::CharacterVirtual::ExtendedUpdateSettings *>(u);
	vchar->ExtendedUpdate(delta, gravity, *updatesettings, bplfilter,
			layfilter, { }, { }, *tempalloc);

	JPH::RVec3 pos = vchar->GetPosition();
	warnx("ply pos: %g %g %g", pos.GetX(), pos.GetY(), pos.GetZ());
}
