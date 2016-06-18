#pragma once
#include "game/entity_id.h"
#include "game/globals/attitude_type.h"
#include "math/vec2.h"

std::vector<entity_id> guns_wielded(entity_id);
float assess_projectile_velocity_of_weapon(entity_id weapon);

void unset_input_flags_of_orphaned_entity(entity_id);

struct identified_danger {
	float amount = 0.f;
	entity_id danger;
	vec2 recommended_evasion;
};

identified_danger assess_danger(entity_id victim, entity_id danger);
attitude_type calculate_attitude(entity_id targeter, entity_id target);
