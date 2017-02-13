#pragma once

namespace messages {
	struct health_event;
}

#include "game/transcendental/step_declaration.h"

class sentience_system {
	void consume_health_event(messages::health_event, const logic_step) const;

public:
	void cast_spells(const logic_step) const;

	void apply_damage_and_generate_health_events(const logic_step) const;
	void cooldown_aimpunches(const logic_step) const;
	void set_borders(const logic_step) const;
	void regenerate_values_and_advance_spell_logic(const logic_step) const;
};