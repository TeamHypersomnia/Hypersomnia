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
			
							if (change.msg == window::event::message::rdown
								|| change.msg == window::event::message::rdoubleclick
								) {
								if (rect_world.held_rect_is_dragged) {
									step.transient.messages.post(item_slot_transfer_request_data{ item_entity, cosmos[inventory_slot_id()], dragged_charges });
									fetched = true;
								}
							}
			
							if (change.msg == window::event::message::wheel) {
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

			int current_x = screen_size.x / 2 - total_width / 2;

			for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
				auto& hb = element.hotbar_buttons[i];
				const auto bbox = hb.get_bbox(root);
				
				hb.rc = xywh(current_x, screen_size.y - max_height - 50, bbox.x, max_height);

				current_x += bbox.x - 1;
			}

			transfers.clear();
		}
	}
}