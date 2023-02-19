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

		if (consider_recoil_position)
		{
			considered_base_offset += recoil.position;
		}
		
		considered_base_offset.rotate(recoil.rotation);

		return considered_base_offset;
	}

	return vec2::zero;
}
