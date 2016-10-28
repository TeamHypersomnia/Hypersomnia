#include "gui_system.h"
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

void gui_system::switch_to_gui_mode_and_back(fixed_step& step) {
	auto& intents = step.messages.get_queue<messages::intent_message>();
	auto& cosmos = step.cosm;

	for (auto& i : intents) {
		auto subject = cosmos[i.subject];

		if (subject.has<components::gui_element>()) {
			auto& gui = subject.get<components::gui_element>();

			if (i.intent == intent_type::SWITCH_TO_GUI && i.pressed_flag) {
				gui.is_gui_look_enabled = !gui.is_gui_look_enabled;
			}

			if (i.intent == intent_type::START_PICKING_UP_ITEMS) {
				//preview_due_to_item_picking_request = i.pressed_flag;
			}
		}

	}
}

void gui_system::advance_gui_elements(fixed_step& step) {
	auto& cosmos = step.cosm;

	for (const auto& root : cosmos.get(processing_subjects::WITH_GUI_ELEMENT)) {
		auto& element = root.get<components::gui_element>();

		if (root.has<components::item_slot_transfers>()) {
			components::gui_element::dispatcher_context context(step, root, element);

			gui_element_location root_location;
			root_location.set(root_of_inventory_gui_location());

			element.rect_world.perform_logic_step(context, root_location, step.get_delta());
			element.rect_world.build_tree_data_into_context(context, root_location);

			const auto& entropies = step.entropy.entropy_per_entity;
			const auto& inputs_for_this_element = entropies.find(root);

			auto& rect_world = element.rect_world;

			if (inputs_for_this_element != entropies.end()) {
				for (const auto& e : (*inputs_for_this_element).second) {
					if (e.has_event_for_gui) {
						bool fetched = false;
						const auto& change = e.event_for_gui;
						const auto& held_rect = rect_world.rect_held_by_lmb;

						if (held_rect.is<item_button_for_item_component_location>()) {
							const auto& item_entity = cosmos[held_rect.get<item_button_for_item_component_location>().item_id];
							auto& dragged_charges = element.dragged_charges;

							if (change.msg == window::event::message::rdown
								|| change.msg == window::event::message::rdoubleclick
								) {
								if (rect_world.held_rect_is_dragged) {
									perform_transfer({ item_entity, cosmos[inventory_slot_id()], dragged_charges }, step);
									fetched = true;
								}
							}

							if (change.msg == window::event::message::wheel) {
								auto& item = item_entity.get<components::item>();

								auto delta = change.scroll.amount;

								dragged_charges += delta;

								if (dragged_charges <= 0)
									dragged_charges = item.charges + dragged_charges;
								if (dragged_charges > item.charges)
									dragged_charges = dragged_charges - item.charges;

							}
						}

						if (!fetched)
							element.rect_world.consume_raw_input_and_generate_gui_events(context, root_location, e.event_for_gui);
					}
				}
			}

			rect_world.call_idle_mousemotion_updater(context, root_location);
		}
	}
}