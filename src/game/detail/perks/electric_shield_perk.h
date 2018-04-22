#pragma once
#include "augs/pad_bytes.h"
#include "game/detail/perks/perk_structs.h"

struct electric_shield_perk;

struct electric_shield_perk_instance {
	using meta_type = electric_shield_perk;

	// GEN INTROSPECTOR struct electric_shield_perk_instance
	perk_timing timing;
	// END GEN INTROSPECTOR
};

struct electric_shield_perk {
	using instance = electric_shield_perk_instance;
	// GEN INTROSPECTOR struct electric_shield_perk
	perk_appearance appearance;
	// END GEN INTROSPECTOR
};
