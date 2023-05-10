#pragma once

namespace messages {
	struct health_event;
	struct damage_message;
}

#include "game/cosmos/step_declaration.h"

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