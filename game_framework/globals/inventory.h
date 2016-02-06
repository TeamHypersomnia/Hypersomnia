#pragma once

enum item_category {
	//ITEM = 1 << 0,
	RAIL_ATTACHMENT = 1 << 1,
	BARREL_ATTACHMENT = 1 << 1,
	SHOULDER_CONTAINER = 1 << 2,
};

enum class slot_function {
	ITEM_DEPOSIT,

	PRIMARY_HAND,
	SECONDARY_HAND,
	
	SHOULDER_SLOT
};
