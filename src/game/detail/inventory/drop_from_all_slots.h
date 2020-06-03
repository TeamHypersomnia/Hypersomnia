#pragma once
#include "game/detail/inventory/perform_transfer.h"

template <class F, class E>
void drop_from_all_slots(const invariants::container& container, const E handle, const impulse_mults impulse, F result_callback) {
	auto& cosm = handle.get_cosmos();

	for (const auto& s : container.slots) {
		const auto items = handle[s.first].get_items_inside();

		for (const auto item : items) {
			auto drop_item = [&](const auto& dropped_item) {
				result_callback(perform_transfer_no_step(item_slot_transfer_request::drop(dropped_item, impulse), cosm));
			};

			if (s.first == slot_function::PERSONAL_DEPOSIT) {
				const auto depoed_items = cosm[item][slot_function::ITEM_DEPOSIT].get_items_inside();

				for (const auto depoed_item : depoed_items) {
					drop_item(depoed_item);
				}
			}
			else {
				drop_item(item);
			}
		}
	}
}

void drop_from_all_slots(const invariants::container& container, const entity_handle handle, const impulse_mults impulse, const logic_step step);
