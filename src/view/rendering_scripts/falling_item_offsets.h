#pragma once
#include <algorithm>
#include "augs/math/vec2.h"
#include "augs/math/arithmetical.h"
#include "augs/templates/remove_cref.h"
#include "game/detail/inventory/item_falling.h"
#include "game/detail/explosive/like_explosive.h"

const vec2 MISSILE_SHADOW_OFFSET = vec2(20, 25);

/*
	Distance of the shadow of an item lying on the floor while still drawn as flying.
*/

constexpr float FALLEN_ITEM_SHADOW_DISTANCE = 4.0f;

/*
	If true, thrown explosives are drawn raised above their shadows, towards the sun,
	with the shadows where the explosives actually are.
*/

constexpr bool RAISE_THROWN_EXPLOSIVES_ABOVE_SHADOWS = true;

/*
	With sun shadows on, character and bullet shadows keep their lengths
	but point along the sun, so that all shadows fall the same way.
*/

inline vec2 calc_along_the_sun(const vec2 legacy_offset, const vec2 sun_step, const bool sun_shadows) {
	if (!sun_shadows || sun_step.is_zero()) {
		return legacy_offset;
	}

	return vec2(sun_step).normalize() * legacy_offset.length();
}

/*
	Offsets from where a flying item actually is.
*/

struct flying_item_offsets {
	vec2 shadow;
	vec2 sprite;
};

/*
	Thrown melee weapons and grenades in flight cast shadows like bullets.
	Falling items' shadows shorten until they hit the floor, down to one right under them,
	and grow again as they bounce off it.

	Normally the item is drawn where it is and its shadow further along the sun.
	Raised explosives are the other way around - their shadow marks where they actually are,
	and they are drawn above it, towards the sun. Released from the hand,
	they start rising from exactly where they are.

	missile_shadow_offset is MISSILE_SHADOW_OFFSET along the sun. now_secs may be interpolated.
*/

template <class E>
flying_item_offsets calc_flying_item_offsets(const E& typed_item, const double now_secs, const vec2 missile_shadow_offset) {
	using item_type = remove_cref<E>;

	const bool raised = RAISE_THROWN_EXPLOSIVES_ABOVE_SHADOWS && ::is_like_thrown_explosive(typed_item);

	const auto longest = missile_shadow_offset.length();
	const auto shortest = std::min(longest, FALLEN_ITEM_SHADOW_DISTANCE);

	const auto hand_height = [&]() {
		if (raised && longest > shortest) {
			return -shortest / (longest - shortest);
		}

		return EXPLOSIVE_HAND_HEIGHT;
	}();

	const auto fall_height = [&]() {
		if constexpr(item_type::template has<components::item>()) {
			const auto& fall = typed_item.template get<components::item>().get_fall();

			if (fall.floor_hits_left > 0) {
				return ::calc_item_fall_height(typed_item, now_secs, hand_height);
			}

			if (fall.when_landed.was_set()) {
				/*
					Rolling on the floor after the last hit, like armed grenades.
				*/

				return 0.0f;
			}
		}

		return 1.0f;
	}();

	const auto shadow_offset = vec2(missile_shadow_offset).normalize() * augs::interp(shortest, longest, fall_height);

	if (raised) {
		return { vec2::zero, -shadow_offset };
	}

	return { shadow_offset, vec2::zero };
}
