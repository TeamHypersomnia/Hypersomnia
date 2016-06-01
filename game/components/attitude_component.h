#pragma once
#include <set>
#include "entity_system/entity_id.h"
#include "game/globals/relations.h"

namespace components {
	struct attitude {
		unsigned parties = 0;
		unsigned hostile_parties = 0;
		std::set<augs::entity_id> specific_hostile_entities;
		
		augs::entity_id chosen_target;
		attitude_type target_attitude = attitude_type::NEUTRAL;

		double maximum_divergence_angle_before_shooting = 10.0;
	};
}
