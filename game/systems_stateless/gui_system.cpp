#include "gui_system.h"
#include "game/detail/gui/game_gui_element_location.h"
#include "game/components/gui_element_component.h"
#include "augs/graphics/renderer.h"

#include "game/components/item_component.h"

#include "game/systems_stateless/crosshair_system.h"
#include "game/transcendental/entity_handle.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/item_component.h"

#include "augs/gui/rect_world.h"
#include "game/detail/gui/immediate_hud.h"
#include "game/enums/slot_function.h"
#include "game/detail/gui/root_of_inventory_gui.h"

#include "game/detail/inventory_utils.h"

#include "augs/gui/button_corners.h"

static char intent_to_hotbar_index(const intent_type type) {
	switch (type) {
	case intent_type::HOTBAR_BUTTON_1: return 1;
	case intent_type::HOTBAR_BUTTON_2: return 2;
	case intent_type::HOTBAR_BUTTON_3: return 3;
	case intent_type::HOTBAR_BUTTON_4: return 4;
	case intent_type::HOTBAR_BUTTON_5: return 5;
	case intent_type::HOTBAR_BUTTON_6: return 6;
	case intent_type::HOTBAR_BUTTON_7: return 7;
	case intent_type::HOTBAR_BUTTON_8: return 8;
	case intent_type::HOTBAR_BUTTON_9: return 9;
	case intent_type::HOTBAR_BUTTON_0: return 0;
	default: return -1;
	}
}

void gui_system::handle_hotbar_and_action_button_presses(logic_step& step) {
	const auto& intents = step.transient.messages.get_queue<messages::intent_message>();
	auto& cosmos = step.cosm;

	for (const auto& i : intents) {
		const auto hotbar_index = intent_to_hotbar_index(i.intent);
		
		if (hotbar_index >= 0) {
			const auto subject = cosmos[i.subject];
			auto* const maybe_gui_element = subject.find<components::gui_element>();

			if (maybe_gui_element != nullptr) {
				auto& gui = *maybe_gui_element;

				auto& currently_held_index = gui.currently_held_hotbar_index;

				if (gui.hotbar_buttons[currently_held_index].get_assigned_entity(subject).dead()) {
					currently_held_index = -1;
				}

				if (gui.hotbar_buttons[hotbar_index].get_assigned_entity(subject).alive()) {
					if (i.is_pressed) {
						const bool should_dual_wield = currently_held_index > -1;

						if (should_dual_wield) {
							const auto setup = components::gui_element::get_setup_from_button_indices(subject, currently_held_index, hotbar_index);
							components::gui_element::apply_and_save_hotbar_selection_setup(step, setup, subject);
							gui.push_setup_when_index_released = -1;
						}
						else {
							const auto setup = components::gui_element::get_setup_from_button_indices(subject, hotbar_index);
							components::gui_element::apply_hotbar_selection_setup(step, setup, subject);
							gui.push_setup_when_index_released = hotbar_index;
						}

						currently_held_index = hotbar_index;
					}
					else {
						if (hotbar_index == currently_held_index) {
							currently_held_index = -1;
						}

						if (hotbar_index == gui.push_setup_when_index_released) {
							const auto setup = components::gui_element::get_setup_from_button_indices(subject, hotbar_index);
							gui.push_setup(setup);

							gui.push_setup_when_index_released = -1;
						}
					}
				}
			}
		}
		else if (i.intent == intent_type::PREVIOUS_HOTBAR_SELECTION_SETUP && i.is_pressed) {
			const auto subject = cosmos[i.subject];

			if (subject.has<components::gui_element>()) {
				components::gui_element::apply_previous_hotbar_selection_setup(step, subject);
			}
		}
	}
}

void gui_system::switch_to_gui_mode_and_back(logic_step& step) {
	const auto& intents = step.transient.messages.get_queue<messages::intent_message>();
	auto& cosmos = step.cosm;

	for (const auto& i : intents) {
		const auto subject = cosmos[i.subject];

		if (subject.has<components::gui_element>()) {
			auto& gui = subject.get<components::gui_element>();

			if (i.intent == intent_type::SWITCH_TO_GUI && i.is_pressed) {
				gui.is_gui_look_enabled = !gui.is_gui_look_enabled;
			}

			if (i.intent == intent_type::START_PICKING_UP_ITEMS) {
				//preview_due_to_item_picking_request = i.is_pressed;
			}
		}
	}
}

void gui_system::advance_gui_elements(logic_step& step) {
	auto& cosmos = step.cosm;

	for (const auto root : cosmos.get(processing_subjects::WITH_GUI_ELEMENT)) {
		auto& element = root.get<components::gui_element>();
		auto& rect_world = element.rect_world;
		const auto screen_size = element.get_screen_size();

		if (root.has<components::item_slot_transfers>()) {
			game_gui_rect_tree tree;
			augs::gui::gui_entropy<game_gui_element_location> entropy;
			
			root_of_inventory_gui root_of_gui(screen_size);

			logic_gui_context context(step, root, tree, root_of_gui);

			root_of_inventory_gui_in_context root_location;

			rect_world.build_tree_data_into_context(context, root_location);

			const auto& entropies = step.entropy.entropy_per_entity;
			const auto& inputs_for_this_element = entropies.find(root);

			if (inputs_for_this_element != entropies.end()) {
				for (const auto& e : (*inputs_for_this_element).second) {
					//if (!element.is_gui_look_enabled) {
					//	ensure(!e.has_event_for_gui);
					//}

					if (e.has_event_for_gui) {
						bool fetched = false;
						const auto& change = e.event_for_gui;
						const auto held_rect = context._dynamic_cast<item_button_in_item>(rect_world.rect_held_by_lmb);
			
						if (held_rect != nullptr) {
							const auto& item_entity = cosmos[held_rect.get_location().item_id];
							auto& dragged_charges = element.dragged_charges;
			
							if (change.msg == augs::window::event::message::rdown
								|| change.msg == augs::window::event::message::rdoubleclick
								) {
								if (rect_world.held_rect_is_dragged) {
									step.transient.messages.post(item_slot_transfer_request_data{ item_entity, cosmos[inventory_slot_id()], dragged_charges });
									fetched = true;
								}
							}
			
							if (change.msg == augs::window::event::message::wheel) {
								const auto& item = item_entity.get<components::item>();
			
								const auto delta = change.scroll.amount;
			
								dragged_charges += delta;
			
								if (dragged_charges <= 0) {
									dragged_charges = item.charges + dragged_charges;
								}
								if (dragged_charges > item.charges) {
									dragged_charges = dragged_charges - item.charges;
								}
							}
						}
			
						if (!fetched) {
							rect_world.consume_raw_input_and_generate_gui_events(context, root_location, e.event_for_gui, entropy);
						}
					}
				}
			}
			
			// rect_world.call_idle_mousemotion_updater(context, root_location, entropy);
			rect_world.advance_elements(context, root_location, entropy, cosmos.get_fixed_delta());

			auto& transfers = step.transient.messages.get_queue<item_slot_transfer_request_data>();

			for (const auto& t : transfers) {
				perform_transfer(cosmos[t], step);
			}

			rect_world.rebuild_layouts(context, root_location);

			int max_height = 0;
			int total_width = 0;

			for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
				const auto& hb = element.hotbar_buttons[i];

				const auto bbox = hb.get_bbox(root);
				max_height = std::max(max_height, bbox.y);

				total_width += bbox.x;
			}

			const int left_rc_spacing = 2;
			const int right_rc_spacing = 1;

			int current_x = screen_size.x / 2 - total_width / 2 - left_rc_spacing;

			auto set_rc = [&](auto& hb) {
				const auto bbox = hb.get_bbox(root);

				hb.rc = xywh(xywhi(current_x, screen_size.y - max_height - 50, bbox.x + left_rc_spacing + right_rc_spacing, max_height));

				current_x += bbox.x + left_rc_spacing + right_rc_spacing;
			};

			for (size_t i = 1; i < element.hotbar_buttons.size(); ++i) {
				set_rc(element.hotbar_buttons[i]);
			}

			set_rc(element.hotbar_buttons[0]);

			transfers.clear();
		}
	}
}