#pragma once
#include "augs/misc/enum_boolset.h"

enum class item_category {
	GENERAL,

	RAIL_ATTACHMENT,
	MUZZLE_ATTACHMENT,
	SHOULDER_CONTAINER,
	TORSO_ARMOR,
	MAGAZINE,
	SHOT_CHARGE,
	ROCKET,

	ARM_BACK,
	ARM_FRONT,

	COUNT
};

typedef augs::enum_boolset<item_category> item_category_flagset;

bool is_clothing(const item_category_flagset& category);