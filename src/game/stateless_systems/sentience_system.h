#pragma once

namespace messages {
	struct health_event;
	struct damage_message;
}

#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"
#include "augs/math/vec2.h"

class sentience_system {
public:
	messages::health_event process_health_event(messages::health_event, const logic_step) const;
	void process_and_post_health_event(messages::health_event, const logic_step) const;
	void process_damage_message(const messages::damage_message&, const logic_step) const;

	void process_special_results_of_health_events(const logic_step) const;

	void cast_spells(const logic_step) const;

	void process_damages_and_generate_health_events(const logic_step) const;
	void cooldown_aimpunches(const logic_step) const;
	void regenerate_values_and_advance_spell_logic(const logic_step) const;
	void rotate_towards_crosshairs_and_driven_vehicles(const logic_step) const;
};

class allocate_new_entity_access;

/* Blood splatter spawning functions */
void spawn_blood_splatters(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_id subject,
	const vec2 position,
	const vec2 impact_direction,
	const real32 damage_amount
);

void spawn_blood_splatters_omnidirectional(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_id subject,
	const vec2 position,
	const real32 damage_amount
);