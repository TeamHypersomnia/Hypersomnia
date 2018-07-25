#pragma once
#include <array>
#include "augs/pad_bytes.h"
#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/components/transform_component.h"

namespace components {
	struct force_joint {
		// GEN INTROSPECTOR struct components::force_joint

		float force_towards_chased_entity = 8000.f;
		float distance_when_force_easing_starts = 10.f;
		float power_of_force_easing_multiplier = 2.f;

		float percent_applied_to_chased_entity = 0.f;

		bool divide_transform_mode = false;
		bool consider_rotation = true;
		pad_bytes<2> pad;

		std::array<vec2, 2> force_offsets;
		// END GEN INTROSPECTOR
	};
}