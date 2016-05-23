#pragma once
#include "entity_system/entity_id.h"
#include "game/globals/relations.h"
#include "math/vec2.h"

void unset_input_flags_of_orphaned_entity(augs::entity_id);

struct identified_danger {
	float amount = 0.f;
	augs::entity_id danger;
	vec2 recommended_evasion;
};

identified_danger assess_danger(augs::entity_id victim, augs::entity_id danger);
attitude_type calculate_attitude(augs::entity_id targeter, augs::entity_id target);