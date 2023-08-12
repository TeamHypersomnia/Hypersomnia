#include "drag_and_drop.h"
#include "slot_button.h"
#include "item_button.h"
#include "augs/templates/identity_templates.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "view/game_gui/elements/gui_grid.h"
#include "view/game_gui/elements/character_gui.h"
#include "game/components/item_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "augs/ensure.h"
#include "view/game_gui/game_gui_context.h"
#include "view/game_gui/game_gui_system.h"
#include "augs/string/format_enum.h"
#include "game/detail/inventory/inventory_utils.h"

void drag_and_drop_callback(
	game_gui_context context, 
	const std::optional<drag_and_drop_result>& drag_result,
	const vec2i total_dragged_amount
) {
	auto& cosm = context.get_cosmos();
	auto& gui = context.get_character_gui();

	if (!drag_result.has_value()) {
		return;
	}

	std::visit([&](const auto& transfer_data) {
		using T = remove_cref<decltype(transfer_data)>;

		if constexpr (std::is_same_v<T, drop_for_item_slot_transfer>) {
			if (transfer_data.result.is_successful()) {
				context.get_game_gui_system().queue_transfer(context.get_subject_entity(), transfer_data.simulated_transfer);
			}
		}
		else if constexpr (std::is_same_v<T, unfinished_drag_of_item>) {
			const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

			if (hotbar_location != nullptr) {
				gui.clear_hotbar_button_assignment(
					hotbar_location.get_location().index
				);
			}
			else {
				const auto& item = cosm[transfer_data.item_id];

				const vec2i griddified = griddify(total_dragged_amount);

				const auto this_button = context.dereference_location(item_button_in_item{ transfer_data.item_id });
				const auto parent_slot = cosm[item.template get<components::item>().get_current_slot()];
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
		else if constexpr (std::is_same_v<T, drop_for_hotbar_assignment>) {
			const auto target_button = context.dereference_location(transfer_data.assign_to);
			ensure(target_button != nullptr);

			const auto target_button_index = target_button.get_location().index;

			const auto new_assigned_item = cosm[transfer_data.item_id];
			const auto owner_transfer_capability = context.get_subject_entity();

			const auto source_hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);
			
			if (source_hotbar_location != nullptr) {
				const auto item_to_be_swapped = target_button->get_assigned_entity(owner_transfer_capability);

				gui.assign_item_to_hotbar_button(target_button_index, owner_transfer_capability, new_assigned_item);
				
				if (item_to_be_swapped.alive()) {
					const auto source_button_index = source_hotbar_location.get_location().index;
					gui.assign_item_to_hotbar_button(source_button_index, owner_transfer_capability, item_to_be_swapped);
				}
			}
			else {
				gui.assign_item_to_hotbar_button(target_button_index, owner_transfer_capability, new_assigned_item);
			}
		}
		else {
			static_assert(always_false_v<T>);
		}
	}, drag_result.value());
}

template <class C>
std::optional<drag_and_drop_result> prepare_drag_and_drop_result(
	const C context, 
	const game_gui_element_location held_rect_id, 
	const game_gui_element_location drop_target_rect_id
) {
	auto access = context.get_access_to_partial_transfers();

	const auto& cosm = context.get_cosmos();
	const auto& element = context.get_character_gui();
	const auto owning_transfer_capability = context.get_subject_entity();

	std::optional<drag_and_drop_result> output;

	auto dragged_item = context.template get_if<item_button_in_item>(held_rect_id);
	const auto dragged_hotbar_button = context.template get_if<hotbar_button_in_character_gui>(held_rect_id);

	hotbar_button_in_character_gui source_hotbar_button_id;

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

			const auto dragged_item_handle = cosm[dragged_item.get_location().item_id];

			const auto target_slot = context.template get_if<slot_button_in_container>(drop_target_rect_id);
			const auto target_drop_item = context.template get_if<drag_and_drop_target_drop_item_in_character_gui>(drop_target_rect_id);
			const auto target_hotbar_button = context.template get_if<hotbar_button_in_character_gui>(drop_target_rect_id);
			auto target_item = context.template get_if<item_button_in_item>(drop_target_rect_id);

			if (target_hotbar_button != nullptr) {
				const auto assigned_entity = target_hotbar_button->get_assigned_entity(owning_transfer_capability);
				const auto hotbar_button_location = target_hotbar_button.get_location();

				drop_for_hotbar_assignment hotbar_drop;
				hotbar_drop.assign_to = hotbar_button_location;
				hotbar_drop.source_hotbar_button_id = source_hotbar_button_id;

				if (assigned_entity == dragged_item_handle) {
					hotbar_drop.hint_text = typesafe_sprintf("Current assignment");
				}
				else {
					if (assigned_entity.dead()) {
						hotbar_drop.hint_text = typesafe_sprintf("Assign %x", hotbar_button_location.index);
					}
					else {
						hotbar_drop.hint_text = typesafe_sprintf("Reassign %x", hotbar_button_location.index);
					}
				}

				hotbar_drop.item_id = dragged_item_handle;

				output = hotbar_drop;
				return output;
			}

			drop_for_item_slot_transfer drop;

			auto& simulated_transfer = drop.simulated_transfer;
			simulated_transfer.item = dragged_item_handle;
			simulated_transfer.params.set_specified_quantity(access, element.dragged_charges == 0 ? -1 : element.dragged_charges);

			drop.source_hotbar_button_id = source_hotbar_button_id;

			bool was_pointing_to_a_stack_target = false;

			if (target_slot != nullptr) {
				simulated_transfer.target_slot = target_slot.get_location().slot_id;
			}
			else if (target_item != nullptr && target_item != dragged_item) {
				const auto target_item_handle = cosm[target_item.get_location().item_id];

				if (target_item_handle.template find<invariants::container>()) {
					const auto compatible_slot = get_slot_with_compatible_category(dragged_item_handle, target_item_handle);

					if (compatible_slot != slot_function::INVALID) {
						simulated_transfer.target_slot = target_item_handle[compatible_slot];
					}
				}
				else if (can_stack_entities(target_item_handle, dragged_item_handle)) {
					simulated_transfer.target_slot = target_item_handle.template get<components::item>().get_current_slot();
					was_pointing_to_a_stack_target = true;
				}
			}
			else if (target_drop_item != nullptr) {
				simulated_transfer.target_slot.unset();
			}
			else {
				possible_target_hovered = false;
			}

			if (possible_target_hovered) {
				if (target_item != nullptr && cosm[simulated_transfer.target_slot].dead()) {
					drop.hint_text = "No compatible slot available";
					drop.result.transferred_charges = 0;
				}
				else {
					drop.result = query_transfer_result(cosm, simulated_transfer);
				}

				const auto predicted_result = drop.result;
				const auto predicted_result_type = predicted_result.result;

				if (predicted_result_type == item_transfer_result_type::THE_SAME_SLOT) {
					drop.hint_text = "Current slot";
				}
				else if (predicted_result.is_successful()) {
					// 	drop.hint_text += "Unmount & ";

					std::string charges_text;
					const auto item_charges = dragged_item_handle.template get<components::item>().get_charges();

					if (item_charges > 1) {
						const auto requested_q = simulated_transfer.params.get_specified_quantity();
						const auto& resulted_q = static_cast<int>(drop.result.transferred_charges);

						if (requested_q == resulted_q) {
							charges_text = " all";
						}
						else {
							charges_text = " " + std::to_string(drop.result.transferred_charges);
							simulated_transfer.params.set_specified_quantity(access, resulted_q);
						}
					}

					if (target_drop_item) {
						drop.hint_text += "Drop" + charges_text + " to ground";
					}
					else if (was_pointing_to_a_stack_target) {
						drop.hint_text += "Stack" + charges_text;
					}
					else {
						switch (simulated_transfer.target_slot.type) {
							case slot_function::ITEM_DEPOSIT: drop.hint_text += "Insert"; break;
							case slot_function::GUN_CHAMBER: drop.hint_text += "Place"; break;
							case slot_function::GUN_CHAMBER_MAGAZINE: drop.hint_text += "Place"; break;
							case slot_function::GUN_DETACHABLE_MAGAZINE: drop.hint_text += "Reload"; break;
							case slot_function::GUN_RAIL: drop.hint_text += "Install"; break;
							case slot_function::TORSO_ARMOR: drop.hint_text += "Wear"; break;
							case slot_function::BACK: drop.hint_text += "Wear"; break;
							case slot_function::OVER_BACK: drop.hint_text += "Wear"; break;
							case slot_function::SHOULDER: drop.hint_text += "Wear"; break;
							case slot_function::PRIMARY_HAND: drop.hint_text += "Wield"; break;
							case slot_function::SECONDARY_HAND: drop.hint_text += "Wield"; break;
							case slot_function::GUN_MUZZLE: drop.hint_text += "Install"; break;
							case slot_function::BELT: drop.hint_text += "Wear"; break;
							default: drop.hint_text += "Unknown slot"; break;
						}

						drop.hint_text += charges_text;
					}
				}
				else {
					switch (predicted_result_type) {
						case item_transfer_result_type::INSUFFICIENT_SPACE: drop.hint_text = "No space"; break;
						case item_transfer_result_type::INVALID_CAPABILITIES: drop.hint_text = "Impossible"; break;
						case item_transfer_result_type::INCOMPATIBLE_CATEGORIES: drop.hint_text = "Incompatible item"; break;
						case item_transfer_result_type::TOO_MANY_ITEMS: drop.hint_text = "Too many items"; break;
						default: drop.hint_text = format_enum(predicted_result_type); break;
					}
				}

				output = drop;
			}
		}

		if (!possible_target_hovered) {
			output = unfinished_drag_of_item{
				source_hotbar_button_id, dragged_item.get_location().item_id
			};
		}
	}

	return output;
}

template std::optional<drag_and_drop_result> prepare_drag_and_drop_result(
	game_gui_context context,
	const game_gui_element_location held_rect_id,
	const game_gui_element_location drop_target_rect_id
);

template std::optional<drag_and_drop_result> prepare_drag_and_drop_result(
	viewing_game_gui_context context,
	const game_gui_element_location held_rect_id,
	const game_gui_element_location drop_target_rect_id
);
