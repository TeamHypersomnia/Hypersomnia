#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"

struct haste_instance {
	augs::stepped_cooldown cast_cooldown;
};

struct haste {
	using instance = haste_instance;

	spell_common_data common;
	spell_appearance appearance;
	unsigned perk_duration_seconds = 0u;
};