#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/misc/value_animator.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct crosshair {
		static vec2 calculate_aiming_displacement(const_entity_handle subject_crosshair, bool snap_epsilon_base_offset = false);

		entity_id character_entity_to_chase;
		vec2 base_offset;
		vec2 bounds_for_base_offset;

		float rotation_offset = 0.f;
		vec2 size_multiplier = vec2(1.0f, 1.0f);
		vec2 sensitivity = vec2(1.0f, 1.0f);

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(character_entity_to_chase), 
				CEREAL_NVP(base_offset), 
				CEREAL_NVP(bounds_for_base_offset), 
				
				CEREAL_NVP(rotation_offset), 
				CEREAL_NVP(size_multiplier), 
				CEREAL_NVP(sensitivity));
		}
	};
}