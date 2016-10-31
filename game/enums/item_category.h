#pragma once

enum class item_category : unsigned {
	//ITEM = 1 << 0,
	RAIL_ATTACHMENT = 1 << 1,
	BARREL_ATTACHMENT = 1 << 2,
	SHOULDER_CONTAINER = 1 << 3,
	TORSO_ARMOR = 1 << 4,
	MAGAZINE = 1 << 5,
	SHOT_CHARGE = 1 << 6,
};