#pragma once

namespace components {
	struct force_joint {
		augs::entity_id chased_entity;

		float force_towards_chased_entity = 8000.f;
		float distance_when_force_easing_starts = 10.f;
		float power_of_force_easing_multiplier = 2.f;
	};
}