#pragma once
#include "augs/padding_byte.h"
#include "perk_timing.h"

struct haste_perk {
	// GEN INTROSPECTOR struct haste_perk
	perk_timing timing;
	bool is_greater = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR
};