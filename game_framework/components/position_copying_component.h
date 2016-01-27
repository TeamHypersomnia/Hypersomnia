#pragma once

#include "entity_system/entity.h"
#include "math/vec2.h"

class position_copying_system;

namespace components {
	struct position_copying  {
		augs::entity_id target;
		
		enum position_copying_type {
			OFFSET,
			ORBIT,
			PARALLAX
		} position_copying_type = position_copying_type::OFFSET;

		vec2 offset;
		vec2 rotation_orbit_offset;
		
		vec2 reference_position;
		vec2 target_reference_position;
		
		float scrolling_speed = 1.0f;

		float rotation_offset = 0.0f;
		float rotation_multiplier = 1.0f;

		bool relative = false;
		bool position_copying_rotation = false;
		bool track_origin = false;

		bool target_newly_set = true;
		bool subscribe_to_previous = false;

		position_copying(augs::entity_id id = augs::entity_id()) {
			set_target(id);
		}

		void set_target(augs::entity_id);

	private:
		friend class position_copying_system;

		vec2 previous;
		float rotation_previous = 0.0f;
	};
}