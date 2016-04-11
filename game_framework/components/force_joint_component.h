#pragma once
#include "misc/constant_size_vector.h"
#include "transform_component.h"

#include "entity_system/entity_id.h"

namespace components {
	struct force_joint {
		augs::entity_id chased_entity;

		float force_towards_chased_entity = 8000.f;
		float distance_when_force_easing_starts = 10.f;
		float power_of_force_easing_multiplier = 2.f;

		float percent_applied_to_chased_entity = 0.f;

		bool divide_transform_mode = false;
		bool consider_rotation = true;
		components::transform chased_entity_offset;

		augs::constant_size_vector<vec2, 2> force_offsets;
	};
}