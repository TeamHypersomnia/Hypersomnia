#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_flavour_id.h"

struct spell_logic_input;

struct electric_triad;

struct electric_triad_instance {
	using meta_type = electric_triad;

	// GEN INTROSPECTOR struct electric_triad_instance
	augs::stepped_cooldown cast_cooldown;
	// END GEN INTROSPECTOR

	bool are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const;
	void perform_logic(const spell_logic_input);
};

struct electric_triad {
	using instance = electric_triad_instance;

	using missile_flavour_type = constrained_entity_flavour_id<
		invariants::rigid_body, 
		invariants::missile, 
		components::sender
	>;

	// GEN INTROSPECTOR struct electric_triad
	spell_common_data common;
	spell_appearance appearance;
	missile_flavour_type missile_flavour;
	// END GEN INTROSPECTOR

	unsigned get_spell_logic_duration_ms() const {
		return 0u;
	}
};