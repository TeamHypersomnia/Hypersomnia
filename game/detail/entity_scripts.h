#pragma once
#include "game/transcendental/entity_id.h"
#include "game/enums/attitude_type.h"
#include "augs/math/vec2.h"
#include "game/transcendental/entity_handle_declaration.h"

float assess_projectile_velocity_of_weapon(const const_entity_handle weapon);

void unset_input_flags_of_orphaned_entity(const entity_handle);

struct identified_danger {
	float amount = 0.f;
	entity_id danger;
	vec2 recommended_evasion;
};

identified_danger assess_danger(
	const const_entity_handle victim, 
	const const_entity_handle danger
);

attitude_type calculate_attitude(
	const const_entity_handle targeter, 
	const const_entity_handle target
);

struct ammunition_information {
	unsigned total_charges = 0;
	float total_ammunition_space_available = 0.f;
	float total_actual_free_space = 0.f;
};

ammunition_information get_ammunition_information(const const_entity_handle handle);