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
#include "game_framework/settings.h"

#include "../shared/inventory_utils.h"

using namespace augs;
using namespace gui;

void game_gui_root::get_member_children(std::vector<augs::gui::rect_id>& children) {
	children.push_back(&inventory_overroot);
	children.push_back(&game_windows_root);
}

gui_system::gui_system(world& parent_world) : processing_system_templated(parent_world) {
	gui.root.children.push_back(&game_gui_root);
	gui.root.clip = false;
	game_gui_root.clip = false;
	game_gui_root.inventory_overroot.clip = false;
	game_gui_root.game_windows_root.clip = false;
}

void gui_system::draw_gui_overlays_for_camera_rendering_request(messages::camera_render_request_message r) {
	if (!is_gui_look_enabled && !preview_due_to_item_picking_request)
		return;

	gui.draw_triangles();
	r.state.output->push_triangles_from_gui_world(gui);

	auto gui_cursor = assets::GUI_CURSOR;
	auto gui_cursor_color = cyan;

	std::wstring tooltip_text = L"";
	
	if (gui.held_rect_is_dragged) {
		item_button* dragged_item = dynamic_cast<item_button*>(gui.rect_held_by_lmb);
		
		if (dragged_item && gui.rect_hovered) {
			slot_button* target_slot = dynamic_cast<slot_button*>(gui.rect_hovered);
			item_button* target_item = dynamic_cast<item_button*>(gui.rect_hovered);

			std::pair<item_transfer_result, slot_function> predicted_result;
			predicted_result.second = slot_function::INVALID;

			bool queried = true;

			if (target_slot)
				predicted_result = { query_transfer_result({ dragged_item->item, target_slot->slot_id }), target_slot->slot_id.type };
			else if (target_item && target_item != dragged_item)
				predicted_result = query_transfer_result(dragged_item->item, target_item->item);
			else
				queried = false;

			if (queried) {
				if (predicted_result.first == item_transfer_result::THE_SAME_SLOT) {
					tooltip_text = L"Current slot";
				}
				else if (predicted_result.first >= item_transfer_result::SUCCESSFUL_TRANSFER) {
					gui_cursor = assets::GUI_CURSOR_ADD;
					gui_cursor_color = green;

					if (predicted_result.first == item_transfer_result::UNMOUNT_BEFOREHAND) {
						tooltip_text += L"Unmount & ";
					}

					switch (predicted_result.second) {
					case slot_function::ITEM_DEPOSIT: tooltip_text = L"Insert"; break;
					case slot_function::GUN_CHAMBER: tooltip_text = L"Place"; break;
					case slot_function::GUN_CHAMBER_MAGAZINE: tooltip_text = L"Place"; break;
					case slot_function::GUN_DETACHABLE_MAGAZINE: tooltip_text = L"Reload"; break;
					case slot_function::GUN_RAIL: tooltip_text = L"Install"; break;
					case slot_function::TORSO_ARMOR_SLOT: tooltip_text = L"Wear"; break;
					case slot_function::SHOULDER_SLOT: tooltip_text = L"Wear"; break;
					case slot_function::PRIMARY_HAND: tooltip_text = L"Pull out"; break;
					case slot_function::SECONDARY_HAND: tooltip_text = L"Pull out"; break;
					case slot_function::GUN_BARREL: tooltip_text = L"Install"; break;
					default: assert(0); break;
					}
				}
				else if (predicted_result.first < item_transfer_result::SUCCESSFUL_TRANSFER) {
					gui_cursor = assets::GUI_CURSOR_ERROR;
					gui_cursor_color = red;

					switch (predicted_result.first) {
					case item_transfer_result::INSUFFICIENT_SPACE: tooltip_text = L"No space"; break;
					case item_transfer_result::INVALID_SLOT_OR_UNOWNED_ROOT: tooltip_text = L"Impossible"; break;
					case item_transfer_result::INCOMPATIBLE_CATEGORIES: tooltip_text = L"Incompatible item"; break;
					case item_transfer_result::NO_SLOT_AVAILABLE: tooltip_text = L"No slot available"; break;
					default: assert(0); break;
					}
				}
			}
		}
	}

	if (is_gui_look_enabled) {
		components::sprite cursor_sprite;
		cursor_sprite.set(gui_cursor);
		cursor_sprite.color = gui_cursor_color;

		shared::state_for_drawing_renderable state;
		state.setup_camera_state(r.state);
		state.screen_space_mode = true;
		state.renderable_transform.pos = gui_crosshair_position;

		cursor_sprite.draw(state);

		if (tooltip_text.size() > 0) {
			tooltip_drawer.set_text(text::format(tooltip_text, text::style()));

			tooltip_drawer.above_left_to_right(gui_crosshair_position);
			
			auto& out = state.output->get_triangle_buffer();

			tooltip_drawer.draw_stroke(out, black);
			tooltip_drawer.draw(out);
		}
	}
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
	game_gui_root.inventory_overroot.cache_descendants_before_children_reassignment();

	std::vector<augs::gui::rect_id> inventory_roots;

	for (auto& root : targets) {
		auto* item_slot_transfers = root->find<components::item_slot_transfers>();
		auto& element = root->get<components::gui_element>();

		if (item_slot_transfers) {
			decltype(element.slot_metadata) new_slot_meta;
			decltype(element.item_metadata) new_item_meta;

			// construct metadata tree to know the already unneeded entries and to see the new ones

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

			new_item_meta[root] = item_button();
			iterate_inventory_tree(root);

			auto& previous_slot_meta = element.slot_metadata;
			auto& previous_item_meta = element.item_metadata;

			// destroy unneeded metadata entries (possibly save elsewhere to preserve drag positions?)

			std::vector<inventory_slot_id> slots_to_erase;
			std::vector<augs::entity_id> items_to_erase;

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

			for (auto& s : slots_to_erase) previous_slot_meta.erase(s);
			for (auto& i : items_to_erase) previous_item_meta.erase(i);

			// construct new metadata entries

			for (auto& new_entry : new_slot_meta) {
				bool new_slot_appeared = previous_slot_meta.find(new_entry.first) == previous_slot_meta.end();

				if (new_slot_appeared) {
					auto& new_slot = new_entry.second;

					new_slot.gui_element_entity = root;
					new_slot.slot_id = new_entry.first;
					new_slot.rc = get_rectangle_for_slot_function(new_entry.first.type);
					new_slot.slot_relative_pos = new_slot.rc.get_position();
					
					if ((DRAW_FREE_SPACE_INSIDE_CONTAINER_ICONS && new_entry.first.type == slot_function::ITEM_DEPOSIT)) {
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
					}
					else {
						new_item.rc.set_position(previous_slot_meta[new_item.item->get<components::item>().current_slot].rc.get_position());
						new_item.rc.set_size(64, 64);
					}

					previous_item_meta.insert(new_entry);
				}
			}

			// construct raw gui rectangle tree from metadata of items and slots 

			for (auto& entry : previous_item_meta) {
				bool is_it_root = entry.first == root;

				if (!is_it_root) {
					auto item_parent = entry.second.item->get<components::item>().current_slot.container_entity;
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
	
	gui.reassign_children_and_unset_invalid_handles(&game_gui_root.inventory_overroot, inventory_roots);
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
		gui.perform_logic_step();
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

		if (i.intent == intent_type::START_PICKING_UP_ITEMS) {
			//preview_due_to_item_picking_request = i.pressed_flag;
		}
	}
}