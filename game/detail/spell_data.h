#pragma once
#include "augs/padding_byte.h"
#include "augs/misc/stepped_timing.h"

struct spell_data {
	unsigned personal_electricity_required = 40;
	unsigned cooldown_ms = 5000;
};

struct spell_instance_data {
	augs::stepped_cooldown cast_cooldown;
};