#pragma once
#include "augs/padding_byte.h"
#include "augs/misc/stepped_timing.h"

struct spell_data {
	augs::stepped_cooldown cast_cooldown;
};