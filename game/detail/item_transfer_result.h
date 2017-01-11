#pragma once
#include "game/enums/item_transfer_result_type.h"

struct item_transfer_result {
	item_transfer_result_type result = item_transfer_result_type::INVALID_RESULT;
	unsigned transferred_charges = 0;
};
