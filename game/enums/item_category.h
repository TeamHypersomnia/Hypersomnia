#pragma once
#include <bitset>
#include "augs/misc/enum_bitset.h"

enum class item_category {
	NONE,
	RAIL_ATTACHMENT,
	MUZZLE_ATTACHMENT,
	SHOULDER_CONTAINER,
	TORSO_ARMOR,
	MAGAZINE,
	SHOT_CHARGE,

	COUNT
};

typedef augs::enum_bitset<item_category> item_category_bitset;