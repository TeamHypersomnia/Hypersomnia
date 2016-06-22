#pragma once
#include <set>
#include "game/entity_id.h"
#include "game/enums/attitude_type.h"

namespace components {
	struct attitude {
		unsigned parties = 0;
		unsigned hostile_parties = 0;
		std::set<entity_id> specific_hostile_entities;
		
		entity_id currently_attacked_visible_entity;
		attitude_type target_attitude = attitude_type::NEUTRAL;

		bool is_alert = false;

		vec2 last_seen_target_position;
		vec2 last_seen_target_velocity;
		bool last_seen_target_position_inspected = false;

		double maximum_divergence_angle_before_shooting = 10.0;
	};
}
