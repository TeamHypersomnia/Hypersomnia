#pragma once
#include "game/enums/item_transfer_result_type.h"

struct containment_result {
	containment_result_type result = containment_result_type::INVALID_RESULT;
	unsigned transferred_charges = 0;
};

struct item_transfer_result {
	item_transfer_result_type result = item_transfer_result_type::INVALID_RESULT;
	unsigned transferred_charges = 0;
};
