#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"

struct fury_of_the_aeons_instance {
	augs::stepped_cooldown cast_cooldown;
};

struct fury_of_the_aeons {
	using instance = fury_of_the_aeons_instance;

	spell_common_data common;
	spell_appearance appearance;
};