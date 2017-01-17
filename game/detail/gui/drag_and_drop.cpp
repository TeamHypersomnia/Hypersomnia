#include "drag_and_drop.h"
#include "slot_button.h"
#include "item_button.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/detail/gui/grid.h"
#include "game/components/gui_element_component.h"
#include "game/components/item_component.h"
#include "game/detail/inventory_utils.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"
#include "augs/ensure.h"
#include "augs/templates/string_templates.h"

void drag_and_drop_callback(
	logic_gui_context context, 
	const drag_and_drop_result& drag_result,
	const vec2i total_dragged_amount
) {
	auto& cosmos = context.get_step().cosm;

	if (drag_result.is<drop_for_item_slot_transfer>()) {
		const auto& transfer_data = drag_result.get<drop_for_item_slot_transfer>();

		if (transfer_data.result.result >= item_transfer_result_type::SUCCESSFUL_TRANSFER) {
			context.get_step().transient.messages.post(transfer_data.simulated_transfer);
		}
	}
	else if (drag_result.is<unfinished_drag_of_item>()) {
		const auto& transfer_data = drag_result.get<unfinished_drag_of_item>();

		const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

		if (hotbar_location != nullptr) {
			components::gui_element::clear_hotbar_button_assignment(
				hotbar_location.get_location().index, 
				context.get_gui_element_entity()
			);
		}
		else {
			const auto& item = cosmos[transfer_data.item_id];

			const vec2i griddified = griddify(total_dragged_amount);

			const auto this_button = context.dereference_location(item_button_in_item{ transfer_data.item_id });
			const auto parent_slot = cosmos[item.get<components::item>().current_slot];
			const auto parent_button = context.dereference_location(slot_button_in_container{ parent_slot.get_id() });

			if (parent_slot->always_allow_exactly_one_item) {
				parent_button->user_drag_offset += griddified;
				parent_button->update_rc(context, parent_button);
			}
			else {
				this_button->drag_offset_in_item_deposit += griddified;
			}
		}
	}
	else if (drag_result.is<drop_for_hotbar_assignment>()) {
		const auto& transfer_data = drag_result.get<drop_for_hotbar_assignment>();

		const auto dereferenced_button = context.dereference_location(transfer_data.assign_to);
		const auto new_assigned_item = cosmos[transfer_data.item_id];
		const auto owner_transfer_capability = context.get_gui_element_entity();

		ensure(dereferenced_button != nullptr);

		const auto source_hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);
		
		if (source_hotbar_location != nullptr) {
			const auto item_to_be_swapped = dereferenced_button->get_assigned_entity(owner_transfer_capability);

			components::gui_element::assign_item_to_hotbar_button(dereferenced_button.get_location().index, owner_transfer_capability, new_assigned_item);
			
			if (item_to_be_swapped.alive()) {
				components::gui_element::assign_item_to_hotbar_button(source_hotbar_location.get_location().index, owner_transfer_capability, item_to_be_swapped);
			}
		}
		else {
			components::gui_element::assign_item_to_hotbar_button(dereferenced_button.get_location().index, owner_transfer_capability, new_assigned_item);
		}
	}
}

template <class C>
drag_and_drop_result prepare_drag_and_drop_result(C context, const game_gui_element_location held_rect_id, const game_gui_element_location drop_target_rect_id) {
	const auto& cosmos = context.get_step().get_cosmos();
	const auto& element = context.get_gui_element_component();
	const auto owning_transfer_capability = context.get_gui_element_entity();

	drag_and_drop_result output;

	auto dragged_item = context._dynamic_cast<item_button_in_item>(held_rect_id);
	const auto dragged_hotbar_button = context._dynamic_cast<hotbar_button_in_gui_element>(held_rect_id);

	hotbar_button_in_gui_element source_hotbar_button_id;

	if (dragged_hotbar_button) {
		source_hotbar_button_id = dragged_hotbar_button.get_location();

		const auto assigned_entity = dragged_hotbar_button->get_assigned_entity(owning_transfer_capability);

		if (assigned_entity.alive()) {
			item_button_in_item dragged_item_location;
			dragged_item_location.item_id = assigned_entity.get_id();

			dragged_item = context.dereference_location(dragged_item_location);
		}
	}

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
				const auto hotbar_button_location = target_hotbar_button.get_location();

				drop_for_hotbar_assignment hotbar_drop;
				hotbar_drop.assign_to = hotbar_button_location;
				hotbar_drop.source_hotbar_button_id = source_hotbar_button_id;

				if (assigned_entity == dragged_item_handle) {
					hotbar_drop.hint_text = typesafe_sprintf(L"Current assignment");
				}
				else {
					if (assigned_entity.dead()) {
						hotbar_drop.hint_text = typesafe_sprintf(L"Assign %x", hotbar_button_location.index);
					}
					else {
						hotbar_drop.hint_text = typesafe_sprintf(L"Reassign %x", hotbar_button_location.index);
					}
				}

				hotbar_drop.item_id = dragged_item_handle;

				output.set(hotbar_drop);
				return output;
			}

			drop_for_item_slot_transfer drop;

			auto& simulated_transfer = drop.simulated_transfer;
			simulated_transfer.item = dragged_item_handle;
			simulated_transfer.specified_quantity = element.dragged_charges == 0 ? -1 : element.dragged_charges;

			drop.source_hotbar_button_id = source_hotbar_button_id;

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
					switch (predicted_result) {
					case item_transfer_result_type::INSUFFICIENT_SPACE: drop.hint_text = L"No space"; break;
					case item_transfer_result_type::INVALID_SLOT_OR_UNOWNED_ROOT: drop.hint_text = L"Impossible"; break;
					case item_transfer_result_type::INCOMPATIBLE_CATEGORIES: drop.hint_text = L"Incompatible item"; break;
					case item_transfer_result_type::NO_SLOT_AVAILABLE: drop.hint_text = L"No slot available"; break;
					default: ensure(0); break;
					}
				}

				output.set(drop);
			}
		}

		if (!possible_target_hovered) {
			output.set(unfinished_drag_of_item{
				source_hotbar_button_id, dragged_item.get_location().item_id
			});
		}
	}

	return output;
}

template drag_and_drop_result prepare_drag_and_drop_result(logic_gui_context context, const game_gui_element_location held_rect_id, const game_gui_element_location drop_target_rect_id);
template drag_and_drop_result prepare_drag_and_drop_result(viewing_gui_context context, const game_gui_element_location held_rect_id, const game_gui_element_location drop_target_rect_id);