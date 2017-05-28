#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"

struct ultimate_wrath_of_the_aeons_instance {
	augs::stepped_cooldown cast_cooldown;
};

struct ultimate_wrath_of_the_aeons {
	using instance = ultimate_wrath_of_the_aeons_instance;

	spell_common_data common;
	spell_appearance appearance;
};