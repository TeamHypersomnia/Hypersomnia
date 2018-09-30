#pragma once
#include "game/enums/item_transfer_result_type.h"

struct containment_result {
	containment_result_type result = containment_result_type::INVALID_RESULT;
	unsigned transferred_charges = 0;
};

struct item_transfer_result {
	item_transfer_result_type result = item_transfer_result_type::INVALID_RESULT;
	capability_relation relation;
	unsigned transferred_charges = 0;
	bool only_initiated_mounting = false;
	bool holster = false;
	bool wield = false;

	bool is_successful() const {
		return result == item_transfer_result_type::SUCCESSFUL_TRANSFER;
	}

	bool is_pickup() const {
		return is_successful() && relation == capability_relation::PICKUP;
	}

	bool is_holster() const {
		return holster;
	}

	bool is_wield() const {
		return wield;
	}
};
