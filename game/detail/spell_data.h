#pragma once
#include "augs/padding_byte.h"
#include "augs/misc/stepped_timing.h"
#include "game/enums/spell_type.h"

#include "augs/misc/constant_size_vector.h"

struct spell_data {
	unsigned personal_electricity_required = 40;
	unsigned cooldown_ms = 5000;
	augs::constant_size_wstring<32> incantation;
};

spell_data get_spell_data(const spell_type);

struct spell_instance_data {
	augs::stepped_cooldown cast_cooldown;
};