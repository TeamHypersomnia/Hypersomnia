#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/misc/value_animator.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct crosshair {
		static vec2 calculate_aiming_displacement(const_entity_handle subject_crosshair, bool snap_epsilon_base_offset = false);

		enum orbit_type {
			NONE,
			ANGLED,
			LOOK
		} orbit_mode = LOOK;

		entity_id character_entity_to_chase;
		vec2 base_offset;
		vec2 bounds_for_base_offset;

		vec2 visible_world_area;
		vec2 max_look_expand;

		float rotation_offset = 0.f;
		vec2 size_multiplier = vec2(1.0f, 1.0f);
		vec2 sensitivity = vec2(1.0f, 1.0f);

		void update_bounds();

		template<class F>
		void for_each_held_id(F f) {
			f(character_entity_to_chase);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(character_entity_to_chase);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(orbit_mode),

				CEREAL_NVP(character_entity_to_chase),
				CEREAL_NVP(base_offset), 
				CEREAL_NVP(bounds_for_base_offset),

				CEREAL_NVP(visible_world_area),
				CEREAL_NVP(max_look_expand),
				
				CEREAL_NVP(rotation_offset), 
				CEREAL_NVP(size_multiplier), 
				CEREAL_NVP(sensitivity));
		}
	};
}