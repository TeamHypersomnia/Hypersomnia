#include "gui_system.h"
#include "graphics/renderer.h"
#include "../assets/texture.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"
#include "../messages/raw_window_input_message.h"

#include "../components/item_component.h"
#include "../components/sprite_component.h"
#include "../components/input_receiver_component.h"
#include "../components/container_component.h"

#include "crosshair_system.h"

void game_gui_root::get_member_children(std::vector<augs::gui::rect_id>& children) {
	children.push_back(&inventory_root);
	children.push_back(&game_windows_root);
}

gui_system::gui_system(world& parent_world) : processing_system_templated(parent_world) {
	gui.root.children.push_back(&game_gui_root);
	gui.root.clip = false;
	game_gui_root.clip = false;
	game_gui_root.inventory_root.clip = false;
	game_gui_root.game_windows_root.clip = false;
}

void gui_system::draw_gui_overlays_for_camera_rendering_request(messages::camera_render_request_message r) {
	if (!is_gui_look_enabled)
		return;

	gui.draw_triangles();
	r.state.output->push_triangles_from_gui_world(gui);

	components::sprite cursor_sprite;
	cursor_sprite.set(assets::texture_id::GUI_CURSOR);
	cursor_sprite.color = cyan;

	shared::state_for_drawing_renderable state;
	state.setup_camera_state(r.state);
	state.screen_space_mode = true;
	state.renderable_transform.pos = gui_crosshair_position;

	cursor_sprite.draw(state);
}

augs::entity_id gui_system::get_game_world_crosshair() {
	auto& crosshairs = parent_world.get_system<crosshair_system>().targets;

	for (auto& c : crosshairs) {
		if (c->is_enabled<components::input_receiver>()) {
			return c;
		}
	}
}

void gui_system::rebuild_gui_tree_based_on_game_state() {
	game_gui_root.inventory_root.cache_descendants_before_children_reassignment();

	std::vector<augs::gui::rect_id> new_inventory_elements;

	for (auto& t : targets) {
		auto* item_slot_transfers = t->find<components::item_slot_transfers>();
		auto& element = t->get<components::gui_element>();

		if (item_slot_transfers) {
			decltype(element.slot_metadata) new_slot_meta;
			decltype(element.item_metadata) new_item_meta;

			std::function<void(augs::entity_id)> iterate_inventory_tree 
				= [&new_slot_meta, &new_item_meta, &iterate_inventory_tree](augs::entity_id container) {
				auto* maybe_container = container->find<components::container>();

				if (maybe_container) {
					/* create new if not found */
					for (auto& s : maybe_container->slots) {
						new_slot_meta[container[s.first]] = slot_button();

						for (auto& i : s.second.items_inside) {
							new_item_meta[i] = item_button();
							iterate_inventory_tree(i);
						}
					}
				}
			};

			iterate_inventory_tree(t);

			auto& slot_meta = element.slot_metadata;
			auto& item_meta = element.item_metadata;

			std::vector<inventory_slot_id> slots_to_erase;
			std::vector<augs::entity_id> items_to_erase;

			for (auto& entry : slot_meta) {
				bool element_was_destroyed = new_slot_meta.find(entry.first) == new_slot_meta.end();

				if (element_was_destroyed)
					slots_to_erase.push_back(entry.first);
			}
			
			for (auto& entry : item_meta) {
				bool element_was_destroyed = new_item_meta.find(entry.first) == new_item_meta.end();

				if (element_was_destroyed)
					items_to_erase.push_back(entry.first);
			}

			for (auto& s : slots_to_erase) slot_meta.erase(s);
			for (auto& i : items_to_erase) item_meta.erase(i);

			for (auto& entry : new_slot_meta) {
				bool new_slot_appeared = slot_meta.find(entry.first) == slot_meta.end();

				if (new_slot_appeared) {
					auto& new_slot = entry.second;
					new_slot.slot_id = entry.first;
					new_slot.rc = get_rectangle_for_slot_function(entry.first.type);
					new_slot.slot_relative_pos = new_slot.rc.get_position();

					slot_meta.insert(entry);
				}
			}

			for (auto& entry : new_item_meta) {
				bool new_item_appeared = item_meta.find(entry.first) == item_meta.end();

				if (new_item_appeared) {
					auto& new_item = entry.second;
					new_item.item = entry.first;
					new_item.rc.set_position(slot_meta[new_item.item->get<components::item>().current_slot].rc.get_position());
					new_item.rc.set_size(64, 64);

					item_meta.insert(entry);
				}
			}

			for (auto& entry : item_meta)
				new_inventory_elements.push_back(&entry.second);

			for (auto& entry : slot_meta)
				new_inventory_elements.push_back(&entry.second);
		}
		
		auto* crosshair = t->find<components::crosshair>();
		
		if (crosshair) {
		
		}
	}
	
	gui.reassign_children_and_unset_invalid_handles(&game_gui_root.inventory_root, new_inventory_elements);
	gui.perform_logic_step();
}

void gui_system::translate_raw_window_inputs_to_gui_events() {
	if (!is_gui_look_enabled)
		return;
	
	auto& window_inputs = parent_world.get_message_queue<messages::raw_window_input_message>();

	for (auto w : window_inputs) {
		if (w.raw_window_input.msg == window::event::mousemotion) {
			gui_crosshair_position += w.raw_window_input.mouse.rel;
			gui_crosshair_position.clamp_from_zero_to(vec2(size.x - 1, size.y - 1));
		}

		w.raw_window_input.mouse.pos = gui_crosshair_position;

		gui.consume_raw_input_and_generate_gui_events(w.raw_window_input);
	}
}

void gui_system::suppress_inputs_meant_for_gui() {
	if (!is_gui_look_enabled)
		return;
	
	auto& intents = parent_world.get_message_queue<messages::unmapped_intent_message>();

	for (auto& it : intents) {
		if (it.intent == intent_type::MOVE_CROSSHAIR ||
			it.intent == intent_type::CROSSHAIR_PRIMARY_ACTION ||
			it.intent == intent_type::CROSSHAIR_SECONDARY_ACTION
			) {
			it.delete_this_message = true;
		}
	}

	parent_world.delete_marked_messages(intents);
}

void gui_system::switch_to_gui_mode_and_back() {
	auto& intents = parent_world.get_message_queue<messages::unmapped_intent_message>();

	for (auto& i : intents) {
		if (i.intent == intent_type::SWITCH_TO_GUI && i.pressed_flag) {
			is_gui_look_enabled = !is_gui_look_enabled;
		}
	}
}