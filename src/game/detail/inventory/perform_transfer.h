#pragma once
#include <optional>

#include "game/messages/queue_destruction.h"
#include "game/messages/item_picked_up_message.h"
#include "game/messages/interpolation_correction_request.h"

#include "game/transcendental/step_declaration.h"
#include "game/components/container_component.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/inventory/inventory_utils.h"

struct perform_transfer_result {
	std::optional<messages::queue_destruction> destructed;
	std::vector<messages::interpolation_correction_request> interpolation_corrected;
	std::optional<messages::item_picked_up_message> picked;

	struct drop {
		sound_effect_input sound_input;
		sound_effect_start_input sound_start;

		components::transform sound_transform;
		entity_id sound_subject;
	};

	std::optional<drop> dropped;

	void notify(logic_step) const;
};

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

template <class F>
void drop_from_all_slots(const components::container& container, const entity_handle handle, F result_callback) {
	for (const auto& s : container.slots) {
		for (const auto item : get_items_inside(handle, s.first)) {
			result_callback(perform_transfer( item_slot_transfer_request{ item, inventory_slot_id() }, handle.get_cosmos()));
		}
	}
}

void drop_from_all_slots(const components::container& container, const entity_handle handle, const logic_step step);

void detail_add_item(const inventory_slot_handle handle, const entity_handle new_item);
void detail_remove_item(const inventory_slot_handle handle, const entity_handle removed_item);
