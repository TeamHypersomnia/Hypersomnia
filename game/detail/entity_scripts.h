#pragma once
#include "game/entity_id.h"
#include "game/enums/attitude_type.h"
#include "math/vec2.h"
#include "game/entity_handle_declaration.h"

std::vector<entity_id> guns_wielded(const_entity_handle);
float assess_projectile_velocity_of_weapon(const_entity_handle weapon);

void unset_input_flags_of_orphaned_entity(entity_handle);

struct identified_danger {
	float amount = 0.f;
	entity_id danger;
	vec2 recommended_evasion;
};

identified_danger assess_danger(const_entity_handle victim, const_entity_handle danger);
attitude_type calculate_attitude(const_entity_handle targeter, const_entity_handle target);
