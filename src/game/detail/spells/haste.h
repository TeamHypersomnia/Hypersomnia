#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"
#include "game/transcendental/entity_handle_declaration.h"

struct spell_logic_input;

struct haste;

struct haste_instance {
	using meta_type = haste;

	// GEN INTROSPECTOR struct haste_instance
	augs::stepped_cooldown cast_cooldown;
	// END GEN INTROSPECTOR

	bool are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const;
	void perform_logic(const spell_logic_input);
};

struct haste {
	using instance = haste_instance;

	// GEN INTROSPECTOR struct haste
	spell_common_data common;
	spell_appearance appearance;
	unsigned perk_duration_seconds = 0u;
	// END GEN INTROSPECTOR

	unsigned get_spell_logic_duration_ms() const {
		return 0u;
	}
};