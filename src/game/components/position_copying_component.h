#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/math/vec2.h"

namespace components {
	struct position_copying {
		enum position_copying_type : unsigned char {
			OFFSET,
			ORBIT,
			PARALLAX
		};

		// GEN INTROSPECTOR struct components::position_copying
		entity_id target;

		vec2 offset;
		vec2 orbit_offset;
		
		vec2 reference_position;
		vec2 target_reference_position;
		
		float scrolling_speed = 1.0f;

		float rotation_offset = 0.0f;
		float rotation_multiplier = 1.0f;

		position_copying_type position_copying_mode = position_copying_type::OFFSET;
		bool position_copying_rotation = false;
		bool track_origin = false;
		bool target_newly_set = true;
		components::transform previous;
		// END GEN INTROSPECTOR
		
		enum class chasing_configuration {
			NONE,
			RELATIVE_ORBIT
		};

		static position_copying configure_chasing(
			const const_entity_handle target,
			const components::transform chaser_place_of_birth,
			const chasing_configuration
		);

		void set_target(const const_entity_handle);
		components::transform get_previous_transform() const;
	};
}