#pragma once
#include "augs/misc/constant_size_vector.h"

struct spell_data {
	unsigned personal_electricity_required = 40u;
	unsigned cooldown_ms = 5000u;
	unsigned casting_time_ms = 0u;
	unsigned perk_duration_seconds = 0u;
	augs::constant_size_wstring<32> incantation;
};