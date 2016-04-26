#pragma once
#include "math/vec2.h"
#include "math/rects.h"
#include "misc/value_animator.h"
#include "entity_system/entity_id.h"

namespace components {
	struct crosshair  {
		//augs::rects::ltrb bounds;
		static vec2 calculate_aiming_displacement(augs::entity_id subject_crosshair, bool snap_epsilon_base_offset = false);

		augs::value_animation blink;
		bool should_blink = false;
		
		augs::entity_id character_entity_to_chase;
		vec2 base_offset;
		vec2 bounds_for_base_offset;

		float rotation_offset = 0.f;
		vec2 size_multiplier = vec2(1.0f, 1.0f);
		vec2 sensitivity = vec2(1.0f, 1.0f);

		crosshair() {
			blink.animators.push_back(augs::value_animator(0.8f, 1.f, 200, augs::value_animator::QUADRATIC));
			blink.animators.push_back(augs::value_animator(1.f, 0.8f, 200, augs::value_animator::QUADRATIC));
		}
	};
}