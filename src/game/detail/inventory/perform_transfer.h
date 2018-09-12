#pragma once
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/components/container_component.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/components/item_component.h"

#include "game/detail/inventory/perform_transfer_result.h"

perform_transfer_result perform_transfer_no_step(
	const item_slot_transfer_request, 
	cosmos& cosm
);

/* Handles all messages in place */

 perform_transfer_result perform_transfer(item_slot_transfer_request, logic_step step);

template <class C, class step_type>
void perform_transfers(const C requests, const step_type step) {
	for (const auto& r : requests) {
		perform_transfer(r, step);
	}
}
