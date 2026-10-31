#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"
#include "game/components/owner_component.h"

/*
	A corpse may move only forward along its axis - the part of its velocity across it is dropped,
	and so is all of it once it would go backwards. The axis is not stored - it follows from the corpse:

	A standing corpse: the direction it was killed in (last_corpse_damage_direction) - it may still turn.
	A lying corpse: the direction its head points in, the way it fell - fixed, as it never rotates.
	A portal turns either to the exit's direction (turn_corpse_to_portal_exit).

	Its velocity is constrained when it is spawned and right before every physics step (constrain_corpses_to_motion_axes) -
	after whatever changed it since: a collision's bounce, bullets, explosions, knives. Only within the physics step
	of a collision itself does the body move as the contact was solved - Box2D integrates the positions right after
	solving the contacts, before any callback could change the velocity. (Until the next step, the game logic
	still sees the bounce's velocity - e.g. a corpse may count as settled a step later.)
*/

inline vec2 project_onto_motion_axis(const vec2 velocity, const vec2 axis) {
	const auto along = velocity.dot(axis);
	return along > 0.f ? axis * along : vec2::zero;
}

inline vec2 calc_lying_corpse_motion_axis(const transformr lying_transform) {
	return -vec2::from_degrees(lying_transform.rotation);
}

template <class E>
bool is_lying_corpse(const E& handle) {
	if (const auto owner = handle.template find<components::owner>()) {
		if (const auto owner_body = handle.get_cosmos()[owner->owner_body]) {
			if (const auto sentience = owner_body.template find<components::sentience>()) {
				return sentience->detached.lying_corpse == handle.get_id();
			}
		}
	}

	return false;
}

template <class E>
std::optional<vec2> find_motion_axis(const E& handle) {
	if (handle.dead()) {
		return std::nullopt;
	}

	if (const auto sentience = handle.template find<components::sentience>()) {
		const auto& direction = sentience->last_corpse_damage_direction;

		if (sentience->is_dead() && !sentience->has_exploded && direction.is_nonzero()) {
			return vec2(direction).normalize();
		}

		return std::nullopt;
	}

	if (::is_lying_corpse(handle)) {
		return ::calc_lying_corpse_motion_axis(handle.get_logic_transform());
	}

	return std::nullopt;
}

template <class E>
void constrain_to_motion_axis(const E& handle) {
	if (const auto axis = ::find_motion_axis(handle)) {
		const auto rigid_body = handle.template get<components::rigid_body>();
		const auto velocity = rigid_body.get_velocity();

		if (const auto constrained = ::project_onto_motion_axis(velocity, *axis); constrained != velocity) {
			rigid_body.set_velocity(constrained);
		}
	}
}

/*
	A portal turns a corpse to the exit's direction - it leaves along it, turned by as much as its way was,
	so it comes out as it went in: a standing corpse by its death direction (and so it later falls that way)
	and its rotation, a lying one by its rotation. The portal must not spin it then -
	Box2D would turn even a body of fixed rotation by its angular velocity.
	Whether it was a corpse turned.
*/

template <class E>
bool turn_corpse_to_portal_exit(const E& handle, transformr& final_transform, const vec2 exit_direction) {
	const auto axis = ::find_motion_axis(handle);

	if (exit_direction.is_zero() || !axis.has_value()) {
		return false;
	}

	const auto exit_axis = vec2(exit_direction).normalize();

	final_transform.rotation += exit_axis.degrees() - axis->degrees();

	if (auto* const sentience = handle.template find<components::sentience>()) {
		sentience->last_corpse_damage_direction = exit_axis;
	}

	return true;
}

/*
	Right before the physics step - after whatever pushed them in the previous step (bullets, explosions, knives).
*/

inline void constrain_corpses_to_motion_axes(cosmos& cosm) {
	cosm.for_each_having<components::sentience>([&](const auto& typed_handle) {
		::constrain_to_motion_axis(typed_handle);
		::constrain_to_motion_axis(cosm[typed_handle.template get<components::sentience>().detached.lying_corpse]);
	});
}

/*
	A standing corpse doesn't bounce off walls either - it would fly back the way it was pushed.
*/

template <class E>
real32 calc_restitution(const E& handle, const invariants::fixtures& colliders_data) {
	if (const auto sentience = handle.template find<components::sentience>()) {
		if (!sentience->is_conscious()) {
			return 0.f;
		}
	}

	return colliders_data.restitution;
}
