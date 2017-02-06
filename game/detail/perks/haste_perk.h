#pragma once
#include "augs/padding_byte.h"
#include "timed_perk.h"

struct haste_perk : timed_perk {
	bool is_greater = false;
	padding_byte pad[3];
};