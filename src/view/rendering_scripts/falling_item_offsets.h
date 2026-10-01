#pragma once
#include <algorithm>
#include "augs/math/vec2.h"
#include "augs/math/arithmetical.h"
#include "augs/graphics/rgba.h"
#include "augs/templates/remove_cref.h"
#include "game/detail/inventory/item_falling.h"
#include "game/detail/explosive/like_explosive.h"
#include "game/components/remnant_component.h"

const vec2 MISSILE_SHADOW_OFFSET = vec2(20, 25);

/*
	Distance of the shadow of an item lying on the floor while still drawn as flying.
*/

constexpr float FALLEN_ITEM_SHADOW_DISTANCE = 4.0f;

/*
	Shells lying on the floor have a bit longer shadows than other fallen items, so they read clearly.
*/

constexpr float FALLEN_SHELL_SHADOW_DISTANCE = 6.0f;
inline const rgba SHELL_SHADOW_COLOR = rgba(0, 0, 0, 200);

/*
	Items thrown, dropped or unmounted from a gun flash white at full intensity
	the moment they start falling, then ease down to nothing over this long.
*/

constexpr double ITEM_THROW_FLASH_SECS = 0.5;

/*
	Thrown explosives cast shadows growing the higher they fly - unphysical with the sun infinitely far,
	but it reads well. The shadow is THROWN_EXPLOSIVE_SHADOW_SCALE_AT_REFERENCE times larger
	when it falls SHADOW_SCALE_REFERENCE_DISTANCE px away from them, linearly in between.
*/

constexpr float THROWN_EXPLOSIVE_SHADOW_SCALE_AT_REFERENCE = 1.5f;
constexpr float SHADOW_SCALE_REFERENCE_DISTANCE = 100.0f;

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
	float shadow_scale = 1.0f;
};

/*
	Thrown melee weapons, grenades and shells in flight cast shadows like bullets.
	Falling items' shadows shorten until they hit the floor, down to one right under them,
	and grow again as they bounce off it.

	Normally the item is drawn where it is and its shadow further along the sun.
	Raised ones - shells - are drawn above where they actually are, towards the sun,
	and their shadow is where it would be if they lay there - so once on the floor, they're drawn right where they are,
	and don't stick into the walls they rest against.
	Released from the hand, they start rising from exactly where they are.

	fall is null for items that never fall.
	missile_shadow_offset is MISSILE_SHADOW_OFFSET along the sun. now_secs may be interpolated.
*/

inline flying_item_offsets calc_flying_offsets(
	const item_fall_state* const fall,
	const bool raised,
	const augs::delta dt,
	const double now_secs,
	const vec2 missile_shadow_offset,
	const float fallen_shadow_distance = FALLEN_ITEM_SHADOW_DISTANCE
) {
	const auto longest = missile_shadow_offset.length();
	const auto shortest = std::min(longest, fallen_shadow_distance);

	const auto hand_height = raised ? 0.0f : EXPLOSIVE_HAND_HEIGHT;

	const auto fall_height = [&]() {
		if (fall != nullptr) {
			if (fall->floor_hits_left > 0) {
				return ::calc_fall_height(*fall, dt, now_secs, hand_height);
			}

			if (fall->when_landed.was_set()) {
				/*
					Lying or rolling on the floor after the last hit.
				*/

				return 0.0f;
			}
		}

		return 1.0f;
	}();

	const auto sun_direction = vec2(missile_shadow_offset).normalize();
	const auto shadow_offset = sun_direction * augs::interp(shortest, longest, fall_height);

	if (raised) {
		const auto lying_shadow_offset = sun_direction * shortest;
		return { lying_shadow_offset, lying_shadow_offset - shadow_offset };
	}

	return { shadow_offset, vec2::zero };
}

inline void scale_shadow_with_distance(flying_item_offsets& offsets, const float scale_at_reference) {
	const auto distance_ratio = offsets.shadow.length() / SHADOW_SCALE_REFERENCE_DISTANCE;
	offsets.shadow_scale = augs::interp(1.0f, scale_at_reference, distance_ratio);
}

/*
	Null for entities that are not items.
*/

template <class E>
const item_fall_state* find_item_fall(const E& typed_item) {
	if constexpr(remove_cref<E>::template has<components::item>()) {
		return std::addressof(typed_item.template get<components::item>().get_fall());
	}
	else {
		return nullptr;
	}
}

template <class E>
flying_item_offsets calc_flying_item_offsets(const E& typed_item, const double now_secs, const vec2 missile_shadow_offset) {
	const auto* const fall = ::find_item_fall(typed_item);

	auto offsets = ::calc_flying_offsets(fall, false, typed_item.get_cosmos().get_fixed_delta(), now_secs, missile_shadow_offset);

	if (::is_like_thrown_explosive(typed_item)) {
		::scale_shadow_with_distance(offsets, THROWN_EXPLOSIVE_SHADOW_SCALE_AT_REFERENCE);
	}

	return offsets;
}

/*
	Fall of shells, which fly once ejected - null for remnants that don't fall.
*/

template <class E>
const item_fall_state* find_shell_fall(const E& typed_remnant) {
	if (const auto remnant = typed_remnant.template find<components::remnant>()) {
		if (remnant->fall.when_started_falling.was_set()) {
			return std::addressof(remnant->fall);
		}
	}

	return nullptr;
}

/*
	shell_fall is null for remnants that don't fall - they get no offsets.
*/

inline flying_item_offsets calc_shell_offsets(
	const item_fall_state* const shell_fall,
	const augs::delta dt,
	const double now_secs,
	const vec2 missile_shadow_offset
) {
	if (shell_fall == nullptr) {
		return {};
	}

	return ::calc_flying_offsets(shell_fall, true, dt, now_secs, missile_shadow_offset, FALLEN_SHELL_SHADOW_DISTANCE);
}
