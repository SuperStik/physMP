#include <err.h>
#include <string.h>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "layers.hpp"
#include "math/angle.h"
#include "math/matrix.h"
#include "player.h"

static void updatetransform(struct player *);

struct player *player_create(struct player *ply, void *p) {
	auto physsys = static_cast<JPH::PhysicsSystem *>(p);
	JPH::CharacterVirtualSettings plysettings;
	plysettings.mShape = new JPH::CapsuleShape();

	auto vchar = new JPH::CharacterVirtual(&plysettings,
			JPH::Vec3(0.0f, 0.0f, 0.0f), JPH::Quat::sIdentity(),
			physsys);

	ctrl_create(&(ply->controller));

	ply->vchar = static_cast<void *>(vchar);

	ply->eyeangles = {0.0f, 0.0f};
	for (int i = 0; i < 3; ++i)
		ply->eyepos[i] = 0.0f;

	return ply;
}

void player_destroy(struct player *ply) {
	delete static_cast<JPH::CharacterVirtual *>(ply->vchar);
	ctrl_destroy(&(ply->controller));
}

void player_turn(struct player *ply, float dx, float dy) {
	ply->eyeangles -= (gvec(float,2)){dy, dx} * 0.1f;

	if (fabsf(ply->eyeangles[0]) > 90.0f)
		ply->eyeangles[0] = copysignf(90.0f, ply->eyeangles[0]);

	ply->eyeangles[1] -= 360.0f * floorf(ply->eyeangles[1] / 360.0f);

	updatetransform(ply);
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

	gvec(float,2) move = ctrl_getmove(&(ply->controller)) * 128.0f;
	float yaw = ply->eyeangles[1];

	float siny, cosy;

	SINCOSPIf(yaw / -180.0f, &siny, &cosy);

	float xrel = move[0] * cosy + move[1] * siny;
	float zrel = move[1] * cosy - move[0] * siny;
	JPH::Vec3 move_vel(xrel, 0.0f, zrel);
	new_velocity += (gravity + move_vel) * delta;

	vchar->SetLinearVelocity(new_velocity);

	auto bplfilter = physsys->GetDefaultBroadPhaseLayerFilter(
			Layers::MOVING);
	auto layfilter = physsys->GetDefaultLayerFilter(Layers::MOVING);
	auto updatesettings = static_cast<const
		JPH::CharacterVirtual::ExtendedUpdateSettings *>(u);
	vchar->ExtendedUpdate(delta, gravity, *updatesettings, bplfilter,
			layfilter, { }, { }, *tempalloc);

	updatetransform(ply);
	JPH::RVec3 eyepos = vchar->GetPosition();
	ply->eyepos[0] = eyepos.GetX();
	ply->eyepos[1] = eyepos.GetY();
	ply->eyepos[2] = eyepos.GetZ();
}

static void updatetransform(struct player *ply) {
	auto vchar = static_cast<JPH::CharacterVirtual *>(ply->vchar);
	if (__builtin_expect(vchar == nullptr, 0))
		return;

	gvec(float,4) rot = ang_eulnoroll2quat(ply->eyeangles[0],
			ply->eyeangles[1]);
	union {
		gvec(float,4) vec[4];
		JPH::Mat44 jph;
	} transform;
	transform.jph = vchar->GetWorldTransform();
	transform.jph.SetTranslation(-transform.jph.GetTranslation());

	gvec(float,4) rotate[4];
	mat_getrotate(rotate, rot);

	gvec(float,4) *viewtransform = ply->transform;
	if (__builtin_expect(viewtransform != nullptr, 1))
		mat_mul(transform.vec, rotate, viewtransform);
}
