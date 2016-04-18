#pragma once

enum item_category {
	//ITEM = 1 << 0,
	RAIL_ATTACHMENT = 1 << 1,
	BARREL_ATTACHMENT = 1 << 2,
	SHOULDER_CONTAINER = 1 << 3,
	TORSO_ARMOR = 1 << 4,
	MAGAZINE = 1 << 5,
	SHOT_CHARGE = 1 << 6,
};

enum class slot_function {
	INVALID,

	ITEM_DEPOSIT,

	GUN_CHAMBER,
	GUN_DETACHABLE_MAGAZINE,
	GUN_CHAMBER_MAGAZINE,
	GUN_RAIL,
	GUN_BARREL,

	PRIMARY_HAND,
	SECONDARY_HAND,
	
	SHOULDER_SLOT,
	TORSO_ARMOR_SLOT
};

enum item_transfer_result_type {
	INVALID_SLOT_OR_UNOWNED_ROOT,

	/* returned by containment_result */
	NO_SLOT_AVAILABLE,

	INCOMPATIBLE_CATEGORIES,
	INSUFFICIENT_SPACE,
	THE_SAME_SLOT,

	SUCCESSFUL_TRANSFER,
	UNMOUNT_BEFOREHAND,
};

struct item_transfer_result {
	item_transfer_result_type result = item_transfer_result_type::SUCCESSFUL_TRANSFER;
	unsigned transferred_charges = 1;
};
