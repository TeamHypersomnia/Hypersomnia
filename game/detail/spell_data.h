#pragma once
#include "augs/padding_byte.h"

struct spell_data {
	float cooldown_remaining_ms = 0.f;
	float last_cooldown_duration_ms = 0.f;
};