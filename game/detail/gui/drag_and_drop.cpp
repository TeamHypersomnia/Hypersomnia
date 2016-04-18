#include "drag_and_drop.h"
#include "game_gui_root.h"
#include "slot_button.h"
#include "item_button.h"
#include "game/messages/item_slot_transfer_request.h"
#include "game/components/item_component.h"
#include "../inventory_utils.h"
#include "entity_system/entity.h"
#include "ensure.h"
#include "stream.h"

bool drag_and_drop_result::will_drop_be_successful() {
	return result.result >= item_transfer_result_type::SUCCESSFUL_TRANSFER;
}

bool drag_and_drop_result::will_item_be_disposed() {
	return result.result >= item_transfer_result_type::SUCCESSFUL_TRANSFER && intent.target_slot.dead();
}

drag_and_drop_result game_gui_world::prepare_drag_and_drop_result() {
	drag_and_drop_result out;
	auto& tooltip_text = out.tooltip_text;

	if (held_rect_is_dragged) {
		auto*& dragged_item = out.dragged_item;

		dragged_item = dynamic_cast<item_button*>(rect_held_by_lmb);

		if (dragged_item && rect_hovered) {
			slot_button* target_slot = dynamic_cast<slot_button*>(rect_hovered);
			item_button* target_item = dynamic_cast<item_button*>(rect_hovered);
			special_drag_and_drop_target* target_special = dynamic_cast<special_drag_and_drop_target*>(rect_hovered);

			out.possible_target_hovered = true;

			messages::item_slot_transfer_request simulated_request;
			simulated_request.item = dragged_item->item;
			simulated_request.specified_quantity = dragged_charges;

			bool was_pointing_to_a_stack_target = false;
			bool no_slot_in_targeted_item = false;

			if (target_slot && target_slot->houted_after_drag_started)
				simulated_request.target_slot = target_slot->slot_id;
			else if (target_item && target_item != dragged_item) {
				if (target_item->item->find<components::container>()) {
					auto compatible_slot = detect_compatible_slot(dragged_item->item, target_item->item);
					
					if (compatible_slot != slot_function::INVALID)
						simulated_request.target_slot = target_item->item[compatible_slot];
					else
						no_slot_in_targeted_item = true;
				}
				else if (can_merge_entities(target_item->item, dragged_item->item)) {
					simulated_request.target_slot = target_item->item->get<components::item>().current_slot;
					was_pointing_to_a_stack_target = true;
				}
				else
					no_slot_in_targeted_item = true;
			}
			else if (target_special) {
				if (target_special->type == special_control::DROP_ITEM)
					simulated_request.target_slot.unset();
			}
			else
				out.possible_target_hovered = false;

			if (out.possible_target_hovered) {
				if (no_slot_in_targeted_item) {
					out.result.result = item_transfer_result_type::NO_SLOT_AVAILABLE;
					out.result.transferred_charges = 0;
				}
				else
					out.result = query_transfer_result(simulated_request);

				out.intent = simulated_request;

				auto predicted_result = out.result.result;

				if (predicted_result == item_transfer_result_type::THE_SAME_SLOT) {
					tooltip_text = L"Current slot";
				}
				else if (predicted_result >= item_transfer_result_type::SUCCESSFUL_TRANSFER) {
					if (predicted_result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
						tooltip_text += L"Unmount & ";
					}

					std::wstring charges_text;
					auto item_charges = dragged_item->item->get<components::item>().charges;

					if (item_charges > 1) {
						if (simulated_request.specified_quantity == out.result.transferred_charges)
							charges_text = L" all";
						else
							charges_text = L" " + augs::to_wstring(out.result.transferred_charges);
					}

					if (target_special) {
						if (target_special->type == special_control::DROP_ITEM) {
							tooltip_text += L"Drop" + charges_text + L" to ground";
						}
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
						case slot_function::GUN_BARREL: tooltip_text += L"Install"; break;
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

	return out;
}