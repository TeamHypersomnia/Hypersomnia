#pragma once
#include "math/vec2.h"
#include "math/rects.h"
#include "misc/value_animator.h"
#include "game/entity_id.h"

namespace components {
	struct crosshair  {
		static vec2 calculate_aiming_displacement(entity_id subject_crosshair, bool snap_epsilon_base_offset = false);

		entity_id character_entity_to_chase;
		vec2 base_offset;
		vec2 bounds_for_base_offset;

		float rotation_offset = 0.f;
		vec2 size_multiplier = vec2(1.0f, 1.0f);
		vec2 sensitivity = vec2(1.0f, 1.0f);
	};
}