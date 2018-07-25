#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/explosions.h"

struct spell_logic_input;

struct ultimate_wrath_of_the_aeons;

struct ultimate_wrath_of_the_aeons_instance {
	using meta_type = ultimate_wrath_of_the_aeons;

	// GEN INTROSPECTOR struct ultimate_wrath_of_the_aeons_instance
	augs::stepped_cooldown cast_cooldown;
	// END GEN INTROSPECTOR

	bool are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const;
	void perform_logic(const spell_logic_input);
};

struct ultimate_wrath_of_the_aeons {
	using instance = ultimate_wrath_of_the_aeons_instance;

	// GEN INTROSPECTOR struct ultimate_wrath_of_the_aeons
	spell_common_data common;
	spell_appearance appearance;
	std::array<standard_explosion_input, 3> explosions;
	particle_effect_input charging_particles;
	sound_effect_input charging_sound;
	// END GEN INTROSPECTOR

	unsigned get_spell_logic_duration_ms() const {
		return 3000u;
	}
};