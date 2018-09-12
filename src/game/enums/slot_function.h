#pragma once
#include <optional>
#include "augs/misc/enum/enum_boolset.h"

constexpr unsigned SPACE_ATOMS_PER_UNIT = 1000;

enum class slot_function {
	INVALID,

	// GEN INTROSPECTOR enum class slot_function
	PERSONAL_DEPOSIT,
	ITEM_DEPOSIT,

	GUN_CHAMBER,
	GUN_DETACHABLE_MAGAZINE,
	GUN_CHAMBER_MAGAZINE,
	GUN_RAIL,
	GUN_MUZZLE,

	PRIMARY_HAND,
	SECONDARY_HAND,

	BELT,
	BACK,
	TORSO_ARMOR,
	HAT,
	// END GEN INTROSPECTOR
	COUNT
};

inline bool is_torso_attachment(const slot_function f) {
	switch (f) {
		case slot_function::PRIMARY_HAND: return true;
		case slot_function::SECONDARY_HAND: return true;

		case slot_function::BELT: return true;
		case slot_function::BACK: return true;
		case slot_function::TORSO_ARMOR: return true;
		case slot_function::HAT: return true;
		default: return false;
	}
};

using slot_flags = augs::enum_boolset<slot_function>;
using optional_slot_flags = std::optional<slot_flags>;