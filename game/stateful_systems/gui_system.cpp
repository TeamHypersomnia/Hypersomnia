#include "gui_system.h"
#include "graphics/renderer.h"

#include "game/entity_id.h"
#include "game/cosmos.h"

#include "game/messages/intent_message.h"

#include "game/components/item_component.h"
#include "game/components/input_receiver_component.h"
#include "game/systems/input_system.h"

#include "game/systems/crosshair_system.h"
#include "game/entity_handle.h"

#include "game/step.h"
#include "game/components/gui_element_component.h"
#include "game/step.h"
#include "game/messages/gui_intents.h"

#include "augs/gui/gui_world.h"
#include "game/detail/gui/game_gui_root.h"
#include "game/detail/gui/immediate_hud.h"
#include "game/enums/slot_function.h"

gui_system::gui_system() {
	gui.root.clip = false;
}

void gui_system::draw_complete_gui_for_camera_rendering_request(viewing_step& r) const {
	r.renderer.push_triangles(gui.draw_triangles());

	if (is_gui_look_enabled)
		gui.draw_cursor_and_tooltip(r);
}

void gui_system::translate_raw_window_inputs_to_gui_events(fixed_step& step) {
	if (!is_gui_look_enabled)
		return;
	
	auto window_inputs = step.entropy.local;

	if (freeze_gui_model()) {
		buffered_inputs_during_freeze.insert(buffered_inputs_during_freeze.end(), window_inputs.begin(), window_inputs.end());
		return;
	}

	window_inputs.insert(window_inputs.begin(), buffered_inputs_during_freeze.begin(), buffered_inputs_during_freeze.end());
	buffered_inputs_during_freeze.clear();
	
	for (auto w : window_inputs)
		gui.consume_raw_input(w);

	gui.perform_logic_step();
}

void gui_system::suppress_inputs_meant_for_gui(fixed_step& step) {
	if (!is_gui_look_enabled)
		return;
	
	erase_remove(step.entropy.local, [](auto it) {
		if (it.msg != window::event::keydown &&
			it.msg != window::event::keyup) {
			return true;
		}

		return false;
	});
}

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

void gui_system::translate_game_events_for_hud(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	hud.acquire_game_events(step);
}

void gui_system::advance_gui_elements(fixed_step& step) {
	auto& cosmos = step.cosm;

	augs::gui::rect parent_of_inventory_controls;

	gui.set_delta_milliseconds(step.get_delta().in_milliseconds());

	gui.root.children = { &game_gui_root };
	parent_of_inventory_controls.cache_descendants_before_children_reassignment();

	std::vector<augs::gui::rect_id> inventory_roots;

	for (auto& root : cosmos.get(processing_subjects::WITH_GUI_ELEMENT)) {
		auto* item_slot_transfers = root.find<components::item_slot_transfers>();
		auto& element = root.get<components::gui_element>();

		if (item_slot_transfers) {
			decltype(element.slot_metadata) new_slot_meta;
			decltype(element.item_metadata) new_item_meta;

			// construct metadata tree to know the already unneeded entries and to see the new ones

			std::function<void(const_entity_handle)> iterate_inventory_tree
				= [&cosmos, &new_slot_meta, &new_item_meta, &iterate_inventory_tree](const_entity_handle container) {
				auto* maybe_container = container.find<components::container>();

				if (maybe_container) {
					/* create new if not found */
					for (auto& s : maybe_container->slots) {
						new_slot_meta[container[s.first]] = slot_button();

						for (auto& i : s.second.items_inside) {
							new_item_meta[i] = item_button();
							iterate_inventory_tree(cosmos[i]);
						}
					}
				}
			};

			new_item_meta[root] = item_button();
			iterate_inventory_tree(root);

			auto& previous_slot_meta = element.slot_metadata;
			auto& previous_item_meta = element.item_metadata;

			// destroy unneeded metadata entries (possibly save elsewhere to preserve drag positions?)

			std::vector<inventory_slot_id> slots_to_erase;
			std::vector<entity_id> items_to_erase;

			for (auto& old_entry : previous_slot_meta) {
				old_entry.second.children.clear();

				bool element_was_destroyed = new_slot_meta.find(old_entry.first) == new_slot_meta.end();

				if (element_was_destroyed)
					slots_to_erase.push_back(old_entry.first);
			}

			for (auto& old_entry : previous_item_meta) {
				old_entry.second.children.clear();

				bool element_was_destroyed = new_item_meta.find(old_entry.first) == new_item_meta.end();

				if (element_was_destroyed)
					items_to_erase.push_back(old_entry.first);
			}

			auto& cached_slot_meta = element.removed_slot_metadata;
			auto& cached_item_meta = element.removed_item_metadata;

			for (auto& s : slots_to_erase) {
				cached_slot_meta[s] = previous_slot_meta[s];
				previous_slot_meta.erase(s);
			}

			for (auto& i : items_to_erase) {
				cached_item_meta[i] = previous_item_meta[i];
				previous_item_meta.erase(i);
			}

			// purge removed metadata from entries with dead entities
			std::vector<inventory_slot_id> dead_slots;
			std::vector<entity_id> dead_items;

			for (auto& s : cached_slot_meta) {
				auto id = s.first;

				if (cosmos[id].dead())
					dead_slots.push_back(id);
			}

			for (auto& i : cached_item_meta) {
				auto id = i.first;

				if (cosmos[id].dead())
					dead_items.push_back(id);
			}

			for (auto& s : dead_slots) cached_slot_meta.erase(s);
			for (auto& i : dead_items) cached_item_meta.erase(i);

			// construct new metadata entries

			for (auto& new_entry : new_slot_meta) {
				bool new_slot_appeared = previous_slot_meta.find(new_entry.first) == previous_slot_meta.end();

				if (new_slot_appeared) {
					auto& new_slot = new_entry.second;

					new_slot.gui_element_entity = root;
					new_slot.slot_id = new_entry.first;
					new_slot.rc = get_rectangle_for_slot_function(new_entry.first.type);
					new_slot.slot_relative_pos = new_slot.rc.get_position();

					if (cached_slot_meta.find(new_entry.first) != cached_slot_meta.end())
						new_slot.user_drag_offset = cached_slot_meta[new_entry.first].user_drag_offset;

					if ((draw_free_space_inside_container_icons && new_entry.first.type == slot_function::ITEM_DEPOSIT)) {
						new_slot.enable_drawing = false;
						new_slot.enable_drawing_of_children = false;
					}

					previous_slot_meta.insert(new_entry);
				}
			}

			for (auto& new_entry : new_item_meta) {
				bool new_item_appeared = previous_item_meta.find(new_entry.first) == previous_item_meta.end();

				if (new_item_appeared) {
					auto& new_item = new_entry.second;
					new_item.gui_element_entity = root;
					new_item.item = new_entry.first;

					if (new_item.is_inventory_root()) {
						new_item.rc.set_position(initial_inventory_root_position());
						new_item.rc.set_size(0, 0);
						new_item.is_container_open = true;
					}
					else {
						new_item.rc.set_position(previous_slot_meta[cosmos[new_item.item].get<components::item>().current_slot].rc.get_position());
						new_item.rc.set_size(64, 64);
					}

					if (cached_item_meta.find(new_entry.first) != cached_item_meta.end())
						new_item.drag_offset_in_item_deposit = cached_item_meta[new_entry.first].drag_offset_in_item_deposit;

					previous_item_meta.insert(new_entry);
				}
			}

			// construct raw gui rectangle tree from metadata of items and slots 

			for (auto& entry : previous_item_meta) {
				bool is_it_root = entry.first == root;

				if (!is_it_root) {
					auto item_parent = cosmos[entry.second.item].get<components::item>().current_slot.container_entity;
					get_meta(item_parent).children.push_back(&entry.second);
				}
			}

			for (auto& entry : previous_slot_meta) {
				auto parent = entry.second.slot_id.container_entity;
				auto& meta = get_meta(parent);
				meta.children.push_back(&entry.second);
			}

			inventory_roots.push_back(&previous_item_meta[root]);
		}
	}

	gui.reassign_children_and_unset_invalid_handles(&game_gui_root.parent_of_inventory_controls, inventory_roots);
	gui.perform_logic_step();

	auto delta = step.get_delta();

	auto timeout_lambda = [delta](const immediate_hud::game_event_visualization& v) {
		return (delta.total_time_passed_in_seconds() - v.time_of_occurence.in_seconds(delta)) > v.maximum_duration_seconds;
	};

	erase_remove(hud.recent_vertically_flying_numbers, timeout_lambda);
	erase_remove(hud.recent_pure_color_highlights, timeout_lambda);
}