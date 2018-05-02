#pragma once
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/container_component.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/components/item_component.h"

perform_transfer_result perform_transfer(
	const item_slot_transfer_request, 
	cosmos& cosm
);

/* Handles all messages in place */

void perform_transfer(
	const item_slot_transfer_request, 
	const logic_step step
);

template <class C, class step_type>
void perform_transfers(const C requests, const step_type step) {
	for (const auto r : requests) {
		perform_transfer(r, step);
	}
}

template <class F, class E>
void drop_from_all_slots(const invariants::container& container, const E handle, const impulse_mults impulse, F result_callback) {
	for (const auto& s : container.slots) {
		for (const auto item : get_items_inside(handle, s.first)) {
			result_callback(perform_transfer(item_slot_transfer_request::drop(item, impulse), handle.get_cosmos()));
		}
	}
}

void drop_from_all_slots(const invariants::container& container, const entity_handle handle, const impulse_mults impulse, const logic_step step);
