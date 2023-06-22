#pragma once
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"

template <class E>
void queue_delete_all_owned_items(const logic_step step, const E handle) {
	handle.for_each_contained_item_recursive(
		[&](const auto& contained) {
			step.queue_deletion_of(entity_id(contained.get_id()), "queue_delete_all_owned_items");
		}
	);
}

template <class F, class E>
void drop_from_all_slots(const invariants::container& container, const E handle, const impulse_mults impulse, F result_callback) {
	auto& cosm = handle.get_cosmos();

	for (auto&& s : container.slots) {
		const auto items = handle[s.first].get_items_inside();

		for (const auto& item : items) {
			auto drop_item = [&](const auto& dropped_item) {
				result_callback(perform_transfer_no_step(item_slot_transfer_request::drop(dropped_item, impulse), cosm));
			};

			if (s.first == slot_function::PERSONAL_DEPOSIT) {
				const auto depoed_items = cosm[item][slot_function::ITEM_DEPOSIT].get_items_inside();

				for (const auto& depoed_item : depoed_items) {
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
