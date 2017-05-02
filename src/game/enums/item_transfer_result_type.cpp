#include "item_transfer_result_type.h"

bool is_successful(const item_transfer_result_type t) {
	return 
		t == item_transfer_result_type::SUCCESSFUL_TRANSFER
		|| t == item_transfer_result_type::SUCCESSFUL_PICKUP
		|| t == item_transfer_result_type::SUCCESSFUL_DROP
	;
}