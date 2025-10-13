#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "player.h"

struct player *player_create(struct player *ply, void *p) {
	JPH::PhysicsSystem *physsys = static_cast<JPH::PhysicsSystem *>(p);
	JPH::CharacterVirtualSettings plysettings;
	JPH::CharacterVirtual *vchar = new JPH::CharacterVirtual(&plysettings,
			JPH::RVec3::sZero(), JPH::Quat::sIdentity(), physsys);

	ply->vchar = static_cast<void *>(vchar);
	ply->eyeangles = {0.0f, 0.0f};
	return ply;
}

void player_destroy(struct player *ply) {
	delete static_cast<JPH::CharacterVirtual *>(ply->vchar);
}
