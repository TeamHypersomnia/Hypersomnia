#pragma once

constexpr unsigned SPACE_ATOMS_PER_UNIT = 1000;

enum class slot_function {
	INVALID,

	// GEN INTROSPECTOR enum class slot_function
	ITEM_DEPOSIT,

	GUN_CHAMBER,
	GUN_DETACHABLE_MAGAZINE,
	GUN_CHAMBER_MAGAZINE,
	GUN_RAIL,
	GUN_MUZZLE,

	PRIMARY_HAND,
	SECONDARY_HAND,

	SHOULDER,
	TORSO_ARMOR,
	HAT,
	// END GEN INTROSPECTOR
	COUNT
};

inline bool is_torso_function(const slot_function s) {
	return s == slot_function::HAT || s == slot_function::SHOULDER || s == slot_function::TORSO_ARMOR || s == slot_function::PRIMARY_HAND || s == slot_function::SECONDARY_HAND;
}