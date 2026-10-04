#pragma once
#include <algorithm>
#include "augs/math/vec2.h"
#include "augs/math/repro_math.h"
#include "augs/math/transform.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"

/*
	A thrown melee weapon bounces off walls like a billiard ball - see make_thrown_melee_bouncing_filter:
	with a circle at its center of mass, without friction and losing no speed in the bounce,
	so it leaves at exactly the angle it came at, and keeps spinning as it did - unless it faces where it flies, see below.

	The circle is THROWN_MELEE_BOUNCE_RADIUS_MULT of the longer side of the weapon as it lies on the ground.
*/

inline constexpr real32 THROWN_MELEE_BOUNCE_RADIUS_MULT = 0.25f;
inline constexpr real32 THROWN_MELEE_BOUNCE_RESTITUTION = 1.f;
inline constexpr real32 THROWN_MELEE_BOUNCE_FRICTION = 0.f;

/*
	Thrown melee weapons not meant to spin - like knives - always face where they fly, like bullets:
	after every physics step, they are turned towards their velocity and stop spinning.
	Thrown, they already face it - after a bounce, they turn towards it smoothly rather than at once,
	by half of the remaining turn every THROWN_MELEE_FACE_FLIGHT_HALF_LIFE_SECS.
*/

inline constexpr real32 THROWN_MELEE_FACE_FLIGHT_HALF_LIFE_SECS = 0.03f;

inline bool faces_flight_direction(const invariants::melee& melee_def) {
	return melee_def.throw_def.throw_angular_speed == 0.f;
}

inline real32 calc_rotation_facing_flight(const real32 current_degrees, const vec2 velocity, const real32 dt_secs) {
	const auto alpha = 1.f - repro::pow(0.5f, dt_secs / THROWN_MELEE_FACE_FLIGHT_HALF_LIFE_SECS);
	return transformr(vec2::zero, current_degrees).interp_rotation(transformr(vec2::zero, velocity.degrees()), alpha);
}

/*
	As big as the body of a thrown item, lying on the ground - see calc_ground_body_scale.
	Computed from the invariants, so that it is known while the item is still held.
*/

template <class E>
real32 calc_thrown_item_body_scale(const E& item) {
	if (const auto item_def = item.template find<invariants::item>()) {
		if (item_def->ground_scale_affects_body) {
			return item_def->ground_scale;
		}
	}

	return 1.f;
}

template <class E>
real32 calc_thrown_melee_bounce_radius(const E& item) {
	const auto size = item.get_logical_size() * ::calc_thrown_item_body_scale(item);
	return std::max(size.x, size.y) * THROWN_MELEE_BOUNCE_RADIUS_MULT;
}
