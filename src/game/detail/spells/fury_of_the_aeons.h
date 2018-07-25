#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/explosions.h"

struct spell_logic_input;

struct fury_of_the_aeons;

struct fury_of_the_aeons_instance {
	using meta_type = fury_of_the_aeons;

	// GEN INTROSPECTOR struct fury_of_the_aeons_instance
	augs::stepped_cooldown cast_cooldown;
	// END GEN INTROSPECTOR

	bool are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const;
	void perform_logic(const spell_logic_input);
};

struct fury_of_the_aeons {
	using instance = fury_of_the_aeons_instance;

	// GEN INTROSPECTOR struct fury_of_the_aeons
	spell_common_data common;
	spell_appearance appearance;
	standard_explosion_input explosion;
	// END GEN INTROSPECTOR

	unsigned get_spell_logic_duration_ms() const {
		return 0u;
	}
};