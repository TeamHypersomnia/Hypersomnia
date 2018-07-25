#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "augs/misc/value_meter.h"

struct spell_logic_input;

struct exaltation;

struct exaltation_instance {
	using meta_type = exaltation;
	// GEN INTROSPECTOR struct exaltation_instance
	augs::stepped_cooldown cast_cooldown;
	// END GEN INTROSPECTOR

	bool are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const;
	void perform_logic(const spell_logic_input);
};

struct exaltation {
	using instance = exaltation_instance;

	// GEN INTROSPECTOR struct exaltation
	spell_common_data common;
	spell_appearance appearance;
	particle_effect_input charging_particles;
	sound_effect_input charging_sound;

	meter_value_type basic_healing_amount = 34;
	// END GEN INTROSPECTOR

	unsigned get_spell_logic_duration_ms() const {
		return 0u;
	}
};