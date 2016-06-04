#pragma once
#include "game/globals/item_transfer_result_type.h"

struct item_transfer_result {
	item_transfer_result_type result = item_transfer_result_type::SUCCESSFUL_TRANSFER;
	unsigned transferred_charges = 1;
};
