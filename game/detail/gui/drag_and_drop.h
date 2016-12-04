#pragma once
#include "game/messages/gui_intents.h"
#include "game/detail/item_transfer_result.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/detail/gui/location_and_pointer.h"
#include "game/detail/gui/item_button.h"
#include "game/detail/inventory_utils.h"

struct drag_and_drop_result {
	item_slot_transfer_request_data simulated_request;
	location_and_pointer<const item_button> dragged_item;
	bool possible_target_hovered = false;
	bool target_slot_alive = false;
	item_transfer_result result;

	bool will_item_be_disposed();
	bool will_drop_be_successful();
	std::wstring tooltip_text;
};

template <class C>
drag_and_drop_result prepare_drag_and_drop_result(C context) {
	const auto& rect_world = context.get_rect_world();
	const auto& cosmos = context.get_step().get_cosmos();
	const auto& element = context.get_gui_element_component();

	drag_and_drop_result out;

	if (rect_world.held_rect_is_dragged) {
		const auto held_rect_id = rect_world.rect_held_by_lmb;
		const auto drop_target_rect_id = rect_world.rect_hovered;

		const auto dragged_item = context._dynamic_cast<const item_button>(held_rect_id);
		out.dragged_item = dragged_item;

		if (dragged_item && context.alive(drop_target_rect_id)) {
			
			const auto dragged_item_handle = cosmos[dragged_item.get_location().item_id];

			const auto target_slot = context._dynamic_cast<const slot_button>(drop_target_rect_id);
			const auto target_item = context._dynamic_cast<const item_button>(drop_target_rect_id);
			const auto target_drop_item = context._dynamic_cast<const drag_and_drop_target_drop_item>(drop_target_rect_id);

			out.possible_target_hovered = true;

			auto& simulated_request = out.simulated_request;
			simulated_request.item = dragged_item_handle;
			simulated_request.specified_quantity = element.dragged_charges;

			bool was_pointing_to_a_stack_target = false;
			bool no_slot_in_targeted_item = false;

			if (target_slot != nullptr && target_slot->houted_after_drag_started) {
				simulated_request.target_slot = target_slot.get_location().slot_id;
			}
			else if (target_item != nullptr && target_item != dragged_item) {
				const auto& target_item_handle = cosmos[target_item.get_location().item_id];

				if (target_item_handle.has<components::container>()) {
					auto compatible_slot = detect_compatible_slot(dragged_item_handle, target_item_handle);

					if (compatible_slot != slot_function::INVALID) {
						simulated_request.target_slot = target_item_handle[compatible_slot];
					}
					else {
						no_slot_in_targeted_item = true;
					}
				}
				else if (can_stack_entities(target_item_handle, dragged_item_handle)) {
					simulated_request.target_slot = target_item_handle.get<components::item>().current_slot;
					was_pointing_to_a_stack_target = true;
				}
				else {
					no_slot_in_targeted_item = true;
				}
			}
			else if (target_drop_item) {
				simulated_request.target_slot.unset();
			}
			else {
				out.possible_target_hovered = false;
			}

			if (out.possible_target_hovered) {
				auto& tooltip_text = out.tooltip_text;

				if (no_slot_in_targeted_item) {
					out.result.result = item_transfer_result_type::NO_SLOT_AVAILABLE;
					out.result.transferred_charges = 0;
				}
				else {
					out.result = query_transfer_result(cosmos[simulated_request]);
				}

				auto predicted_result = out.result.result;

				if (predicted_result == item_transfer_result_type::THE_SAME_SLOT) {
					tooltip_text = L"Current slot";
				}
				else if (predicted_result >= item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					if (predicted_result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
						tooltip_text += L"Unmount & ";
					}

					std::wstring charges_text;
					auto item_charges = dragged_item_handle.get<components::item>().charges;

					if (item_charges > 1) {
						if (simulated_request.specified_quantity == out.result.transferred_charges)
							charges_text = L" all";
						else
							charges_text = L" " + to_wstring(out.result.transferred_charges);
					}

					if (target_drop_item) {
						tooltip_text += L"Drop" + charges_text + L" to ground";
					}
					else if (was_pointing_to_a_stack_target) {
						tooltip_text += L"Stack" + charges_text;
					}
					else {
						switch (simulated_request.target_slot.type) {
						case slot_function::ITEM_DEPOSIT: tooltip_text += L"Insert"; break;
						case slot_function::GUN_CHAMBER: tooltip_text += L"Place"; break;
						case slot_function::GUN_CHAMBER_MAGAZINE: tooltip_text += L"Place"; break;
						case slot_function::GUN_DETACHABLE_MAGAZINE: tooltip_text += L"Reload"; break;
						case slot_function::GUN_RAIL: tooltip_text += L"Install"; break;
						case slot_function::TORSO_ARMOR_SLOT: tooltip_text += L"Wear"; break;
						case slot_function::SHOULDER_SLOT: tooltip_text += L"Wear"; break;
						case slot_function::PRIMARY_HAND: tooltip_text += L"Wield"; break;
						case slot_function::SECONDARY_HAND: tooltip_text += L"Wield"; break;
						case slot_function::GUN_MUZZLE: tooltip_text += L"Install"; break;
						default: ensure(0); break;
						}

						tooltip_text += charges_text;
					}
				}
				else if (predicted_result < item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					switch (predicted_result) {
					case item_transfer_result_type::INSUFFICIENT_SPACE: tooltip_text = L"No space"; break;
					case item_transfer_result_type::INVALID_SLOT_OR_UNOWNED_ROOT: tooltip_text = L"Impossible"; break;
					case item_transfer_result_type::INCOMPATIBLE_CATEGORIES: tooltip_text = L"Incompatible item"; break;
					case item_transfer_result_type::NO_SLOT_AVAILABLE: tooltip_text = L"No slot available"; break;
					default: ensure(0); break;
					}
				}
			}
		}
	}

	out.target_slot_alive = context.get_step().get_cosmos()[out.simulated_request.target_slot].alive();
	return out;
}