#pragma once
#include "3rdparty/Box2D/Dynamics/b2Body.h"
#include "augs/math/vec2.h"
#include "game/cosmos/cosmos.h"
#include "game/components/rigid_body_component.h"

/*
	A body with special_physics::only_moves_along set may move only forward along it -
	the part of its velocity across that direction is dropped, and so is all of it once it would go backwards.

	A standing corpse: the direction it was killed in - it may still turn.
	A lying corpse: the direction its head points in, the way it fell - fixed, as it never rotates.
	Its velocity is constrained when it is spawned, every step (constrain_to_motion_axis of its handle),
	and after the contacts are solved (of its b2Body) - so only the step of a collision itself
	still moves it as solved.
*/

inline vec2 project_onto_motion_axis(const vec2 velocity, const vec2 axis) {
	const auto along = velocity.dot(axis);
	return along > 0.f ? axis * along : vec2::zero;
}

template <class E>
void constrain_to_motion_axis(const E& handle) {
	if (handle.dead()) {
		return;
	}

	const auto rigid_body = handle.template find<components::rigid_body>();

	if (!rigid_body) {
		return;
	}

	const auto axis = rigid_body.get_special().only_moves_along;

	if (axis.is_zero()) {
		return;
	}

	const auto velocity = rigid_body.get_velocity();
	const auto constrained = ::project_onto_motion_axis(velocity, axis);

	if (constrained != velocity) {
		rigid_body.set_velocity(constrained);
	}
}

template <class B>
void constrain_body_to_motion_axis(const cosmos& cosm, B& body) {
	const auto handle = cosm[body.GetUserData()];

	if (handle.dead()) {
		return;
	}

	const auto rigid_body = handle.template find<components::rigid_body>();

	if (!rigid_body) {
		return;
	}

	const auto axis = rigid_body.get_special().only_moves_along;

	if (axis.is_zero()) {
		return;
	}

	const auto velocity = vec2(body.GetLinearVelocity());
	const auto constrained = ::project_onto_motion_axis(velocity, axis);

	if (constrained != velocity) {
		body.SetLinearVelocity(b2Vec2(constrained));
	}
}
