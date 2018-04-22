#pragma once
#include "augs/pad_bytes.h"
#include "game/detail/perks/perk_structs.h"

struct haste_perk;

struct haste_perk_instance {
	using meta_type = haste_perk;
	// GEN INTROSPECTOR struct haste_perk_instance
	perk_timing timing;
	bool is_greater = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR
};

struct haste_perk {
	using instance = haste_perk_instance;
	// GEN INTROSPECTOR struct haste_perk
	perk_appearance appearance;
	// END GEN INTROSPECTOR
};