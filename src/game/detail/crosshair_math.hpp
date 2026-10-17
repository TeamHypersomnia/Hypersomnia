#pragma once
#include "augs/math/vec2.h"
#include "game/enums/melee_fighter_state.h"

template <class E>
vec2 calc_crosshair_displacement(const E& self, bool consider_recoil_position) {
	if (const auto crosshair = self.find_crosshair()) {
		auto considered_base_offset = crosshair->base_offset;
		const auto& recoil = crosshair->recoil;

		const bool snap_epsilon_base_offset = false;

		if (snap_epsilon_base_offset && considered_base_offset.is_epsilon(4)) {
			considered_base_offset.set(4, 0);
		}

		if (consider_recoil_position) {
			considered_base_offset += recoil.position;
		}
		
		considered_base_offset.rotate(recoil.rotation);

		return considered_base_offset;
	}

	return vec2::zero;
}

/*
	Where a character looks - the same for the drawn fog of war, the minimap pins and the bots' sight:
	its crosshair with the recoil, or where its body faces while the crosshair is still zero (right after spawning).
	The viewer's own fog of war adds the crosshair movement not yet applied by a step.
*/

template <class E>
vec2 calc_look_direction(const E& self, const vec2 pending_crosshair_displacement = vec2::zero) {
	const auto dir = ::calc_crosshair_displacement(self) + pending_crosshair_displacement;

	if (dir.is_zero()) {
		if (const auto transform = self.find_logic_transform()) {
			return vec2::from_degrees(transform->rotation);
		}
	}

	return dir;
}
