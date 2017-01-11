#pragma once
#include "augs/gui/dereferenced_location.h"
#include "game/messages/gui_intents.h"
#include "game/detail/item_transfer_result.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/detail/gui/item_button.h"
#include "game/detail/inventory_utils.h"

struct unfinished_drag_of_item {
	entity_id item_id;
};

struct drop_for_item_slot_transfer {
	item_slot_transfer_request_data simulated_transfer;
	item_transfer_result result;

	constant_size_wstring<32> hint_text;
};

struct drop_for_hotbar_assignment {
	entity_id item_id;
	hotbar_button_in_gui_element assign_to;

	constant_size_wstring<20> hint_text;
};

typedef augs::trivial_variant<
	drop_for_item_slot_transfer,
	drop_for_hotbar_assignment,
	unfinished_drag_of_item
> drag_and_drop_result;

template <class C>
drag_and_drop_result prepare_drag_and_drop_result(C context, const game_gui_element_location held_rect_id, const game_gui_element_location drop_target_rect_id) {
	const auto& cosmos = context.get_step().get_cosmos();
	const auto& element = context.get_gui_element_component();
	const auto owning_transfer_capability = context.get_gui_element_entity();

	drag_and_drop_result output;

	const auto dragged_item = context._dynamic_cast<item_button_in_item>(held_rect_id);

	if (dragged_item) {
		bool possible_target_hovered = false;

		if (context.alive(drop_target_rect_id)) {
			possible_target_hovered = true;

			const auto dragged_item_handle = cosmos[dragged_item.get_location().item_id];

			const auto target_slot = context._dynamic_cast<slot_button_in_container>(drop_target_rect_id);
			const auto target_drop_item = context._dynamic_cast<drag_and_drop_target_drop_item_in_gui_element>(drop_target_rect_id);
			const auto target_hotbar_button = context._dynamic_cast<hotbar_button_in_gui_element>(drop_target_rect_id);
			auto target_item = context._dynamic_cast<item_button_in_item>(drop_target_rect_id);

			if (target_hotbar_button != nullptr) {
				const auto assigned_entity = target_hotbar_button->get_assigned_entity(owning_transfer_capability);

				if (assigned_entity.dead()) {
					const auto hotbar_button_location = target_hotbar_button.get_location();

					drop_for_hotbar_assignment hotbar_drop;
					hotbar_drop.assign_to = hotbar_button_location;
					hotbar_drop.hint_text = typesafe_sprintf(L"Assign to %x", hotbar_button_location.index);
					hotbar_drop.item_id = dragged_item_handle;

					output.set(hotbar_drop);
					return output;
				}
				else {
					item_button_in_item assigned_item_location;
					assigned_item_location.item_id = assigned_entity.get_id();

					target_item = context.dereference_location(assigned_item_location);
				}
			}
			
			drop_for_item_slot_transfer drop;

			auto& simulated_transfer = drop.simulated_transfer;
			simulated_transfer.item = dragged_item_handle;
			simulated_transfer.specified_quantity = element.dragged_charges;

			bool was_pointing_to_a_stack_target = false;
			bool no_slot_in_targeted_item = false;

			if (target_slot != nullptr) {
				simulated_transfer.target_slot = target_slot.get_location().slot_id;
			}
			else if (target_item != nullptr && target_item != dragged_item) {
				const auto target_item_handle = cosmos[target_item.get_location().item_id];

				if (target_item_handle.has<components::container>()) {
					const auto compatible_slot = get_slot_with_compatible_category(dragged_item_handle, target_item_handle);

					if (compatible_slot != slot_function::INVALID) {
						simulated_transfer.target_slot = target_item_handle[compatible_slot];
					}
					else {
						no_slot_in_targeted_item = true;
					}
				}
				else if (can_stack_entities(target_item_handle, dragged_item_handle)) {
					simulated_transfer.target_slot = target_item_handle.get<components::item>().current_slot;
					was_pointing_to_a_stack_target = true;
				}
				else {
					no_slot_in_targeted_item = true;
				}
			}
			else if (target_drop_item != nullptr) {
				simulated_transfer.target_slot.unset();
			}
			else {
				possible_target_hovered = false;
			}

			if (possible_target_hovered) {
				if (no_slot_in_targeted_item) {
					drop.result.result = item_transfer_result_type::NO_SLOT_AVAILABLE;
					drop.result.transferred_charges = 0;
				}
				else {
					drop.result = query_transfer_result(cosmos[simulated_transfer]);
				}

				const auto predicted_result = drop.result.result;

				if (predicted_result == item_transfer_result_type::THE_SAME_SLOT) {
					drop.hint_text = L"Current slot";
				}
				else if (predicted_result >= item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					if (predicted_result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
						drop.hint_text += L"Unmount & ";
					}

					std::wstring charges_text;
					auto item_charges = dragged_item_handle.get<components::item>().charges;

					if (item_charges > 1) {
						if (simulated_transfer.specified_quantity == drop.result.transferred_charges)
							charges_text = L" all";
						else
							charges_text = L" " + to_wstring(drop.result.transferred_charges);
					}

					if (target_drop_item) {
						drop.hint_text += L"Drop" + charges_text + L" to ground";
					}
					else if (was_pointing_to_a_stack_target) {
						drop.hint_text += L"Stack" + charges_text;
					}
					else {
						switch (simulated_transfer.target_slot.type) {
						case slot_function::ITEM_DEPOSIT: drop.hint_text += L"Insert"; break;
						case slot_function::GUN_CHAMBER: drop.hint_text += L"Place"; break;
						case slot_function::GUN_CHAMBER_MAGAZINE: drop.hint_text += L"Place"; break;
						case slot_function::GUN_DETACHABLE_MAGAZINE: drop.hint_text += L"Reload"; break;
						case slot_function::GUN_RAIL: drop.hint_text += L"Install"; break;
						case slot_function::TORSO_ARMOR_SLOT: drop.hint_text += L"Wear"; break;
						case slot_function::SHOULDER_SLOT: drop.hint_text += L"Wear"; break;
						case slot_function::PRIMARY_HAND: drop.hint_text += L"Wield"; break;
						case slot_function::SECONDARY_HAND: drop.hint_text += L"Wield"; break;
						case slot_function::GUN_MUZZLE: drop.hint_text += L"Install"; break;
						default: ensure(0); break;
						}

						drop.hint_text += charges_text;
					}
				}
				else if (predicted_result < item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					const bool transfer_drop_was_due_to_hotbar_button = target_hotbar_button != nullptr;

					if (transfer_drop_was_due_to_hotbar_button) {
						drop_for_hotbar_assignment hotbar_drop;
						hotbar_drop.assign_to = target_hotbar_button;
						hotbar_drop.hint_text = typesafe_sprintf(L"Reassign to %x", target_hotbar_button.get_location().index);
						hotbar_drop.item_id = dragged_item_handle;
						
						output.set(hotbar_drop);
						return output;
					}
					else {
						switch (predicted_result) {
						case item_transfer_result_type::INSUFFICIENT_SPACE: drop.hint_text = L"No space"; break;
						case item_transfer_result_type::INVALID_SLOT_OR_UNOWNED_ROOT: drop.hint_text = L"Impossible"; break;
						case item_transfer_result_type::INCOMPATIBLE_CATEGORIES: drop.hint_text = L"Incompatible item"; break;
						case item_transfer_result_type::NO_SLOT_AVAILABLE: drop.hint_text = L"No slot available"; break;
						default: ensure(0); break;
						}
					}
				}

				output.set(drop);
			}
		}
		
		if (!possible_target_hovered) {
			output.set(unfinished_drag_of_item{ dragged_item.get_location().item_id });
		}
	}

	return output;
}