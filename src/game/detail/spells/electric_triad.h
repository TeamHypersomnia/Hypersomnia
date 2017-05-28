#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"

struct electric_triad_instance {
	augs::stepped_cooldown cast_cooldown;
};

struct electric_triad {
	using instance = electric_triad_instance;

	spell_common_data common;
	spell_appearance appearance;
};