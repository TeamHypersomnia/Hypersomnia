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
		};

		// GEN INTROSPECTOR struct components::crosshair
		orbit_type orbit_mode = LOOK;

		child_entity_id recoil_entity;

		entity_id character_entity_to_chase;
		
		vec2 base_offset;
		vec2 base_offset_bound;

		float rotation_offset = 0.f;
		vec2 sensitivity = vec2(1.0f, 1.0f);
		// END GEN INTROSPECTOR

		vec2 get_bounds_in_this_look() const;
	};
}