#include "gui_element_component.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/components/container_component.h"

namespace components {
	gui_element::gui_element() :
		drop_item_icon(augs::gui::material(assets::texture_id::DROP_HAND_ICON, red))
	{
	}

	rects::xywh<float> gui_element::get_rectangle_for_slot_function(const slot_function f) const {
		switch (f) {
		case slot_function::PRIMARY_HAND: return rects::xywh<float>(100, 0, 33, 33);
		case slot_function::SHOULDER_SLOT: return rects::xywh<float>(100, -100, 33, 33);
		case slot_function::SECONDARY_HAND: return rects::xywh<float>(-100, 0, 33, 33);
		case slot_function::TORSO_ARMOR_SLOT: return rects::xywh<float>(0, 0, 33, 33);

		case slot_function::ITEM_DEPOSIT: return rects::xywh<float>(0, -100, 33, 33);

		case slot_function::GUN_DETACHABLE_MAGAZINE: return rects::xywh<float>(0, 50, 33, 33);
		case slot_function::GUN_CHAMBER: return rects::xywh<float>(0, -50, 33, 33);
		case slot_function::GUN_BARREL: return rects::xywh<float>(-50, 0, 33, 33);
		default: ensure(0);
		}
		ensure(0);

		return rects::xywh<float>(0, 0, 0, 0);
	}

	vec2i gui_element::get_initial_position_for_special_control(const special_control s) const {
		switch (s) {
		case special_control::DROP_ITEM: return vec2i(size.x - 150, 30);
		}
	}

	vec2 gui_element::initial_inventory_root_position() const {
		return vec2(size.x - 250, size.y - 200);
	}

	void gui_element::advance(fixed_step& step) {
//		//gui.set_delta_milliseconds(step.get_delta().in_milliseconds());
//
//		auto& cosmos = step.cosm;
//
//		for (auto& root : cosmos.get(processing_subjects::WITH_GUI_ELEMENT)) {
//			auto* item_slot_transfers = root.find<components::item_slot_transfers>();
//			auto& element = root.get<components::gui_element>();
//
//			if (item_slot_transfers) {
//				decltype(element.slot_metadata) new_slot_meta;
//				decltype(element.item_metadata) new_item_meta;
//
//				// construct metadata tree to know the already unneeded entries and to see the new ones
//
//				std::function<void(const_entity_handle)> iterate_inventory_tree
//					= [&cosmos, &new_slot_meta, &new_item_meta, &iterate_inventory_tree](const_entity_handle container) {
//					auto* maybe_container = container.find<components::container>();
//
//					if (maybe_container) {
//						/* create new if not found */
//						for (auto& s : maybe_container->slots) {
//							new_slot_meta[container[s.first]] = slot_button();
//
//							for (auto& i : s.second.items_inside) {
//								new_item_meta[i] = item_button();
//								iterate_inventory_tree(cosmos[i]);
//							}
//						}
//					}
//				};
//
//				new_item_meta[root] = item_button();
//				iterate_inventory_tree(root);
//
//				auto& previous_slot_meta = element.slot_metadata;
//				auto& previous_item_meta = element.item_metadata;
//
//				// destroy unneeded metadata entries (possibly save elsewhere to preserve drag positions?)
//
//				std::vector<inventory_slot_id> slots_to_erase;
//				std::vector<entity_id> items_to_erase;
//
//				for (auto& old_entry : previous_slot_meta) {
//					old_entry.second.children.clear();
//
//					bool element_was_destroyed = new_slot_meta.find(old_entry.first) == new_slot_meta.end();
//
//					if (element_was_destroyed)
//						slots_to_erase.push_back(old_entry.first);
//				}
//
//				for (auto& old_entry : previous_item_meta) {
//					old_entry.second.children.clear();
//
//					bool element_was_destroyed = new_item_meta.find(old_entry.first) == new_item_meta.end();
//
//					if (element_was_destroyed)
//						items_to_erase.push_back(old_entry.first);
//				}
//
//				auto& cached_slot_meta = element.removed_slot_metadata;
//				auto& cached_item_meta = element.removed_item_metadata;
//
//				for (auto& s : slots_to_erase) {
//					cached_slot_meta[s] = previous_slot_meta[s];
//					previous_slot_meta.erase(s);
//				}
//
//				for (auto& i : items_to_erase) {
//					cached_item_meta[i] = previous_item_meta[i];
//					previous_item_meta.erase(i);
//				}
//
//				// purge removed metadata from entries with dead entities
//				std::vector<inventory_slot_id> dead_slots;
//				std::vector<entity_id> dead_items;
//
//				for (auto& s : cached_slot_meta) {
//					auto id = s.first;
//
//					if (cosmos[id].dead())
//						dead_slots.push_back(id);
//				}
//
//				for (auto& i : cached_item_meta) {
//					auto id = i.first;
//
//					if (cosmos[id].dead())
//						dead_items.push_back(id);
//				}
//
//				for (auto& s : dead_slots) cached_slot_meta.erase(s);
//				for (auto& i : dead_items) cached_item_meta.erase(i);
//
//				// construct new metadata entries
//
//				for (auto& new_entry : new_slot_meta) {
//					bool new_slot_appeared = previous_slot_meta.find(new_entry.first) == previous_slot_meta.end();
//
//					if (new_slot_appeared) {
//						auto& new_slot = new_entry.second;
//
//						new_slot.gui_element_entity = root;
//						new_slot.slot_id = new_entry.first;
//						new_slot.rc = get_rectangle_for_slot_function(new_entry.first.type);
//						new_slot.slot_relative_pos = new_slot.rc.get_position();
//
//						if (cached_slot_meta.find(new_entry.first) != cached_slot_meta.end())
//							new_slot.user_drag_offset = cached_slot_meta[new_entry.first].user_drag_offset;
//
//						if ((draw_free_space_inside_container_icons && new_entry.first.type == slot_function::ITEM_DEPOSIT)) {
//							new_slot.enable_drawing = false;
//							new_slot.enable_drawing_of_children = false;
//						}
//
//						previous_slot_meta.insert(new_entry);
//					}
//				}
//
//				for (auto& new_entry : new_item_meta) {
//					bool new_item_appeared = previous_item_meta.find(new_entry.first) == previous_item_meta.end();
//
//					if (new_item_appeared) {
//						auto& new_item = new_entry.second;
//						new_item.gui_element_entity = root;
//						new_item.item = new_entry.first;
//
//						if (new_item.is_inventory_root()) {
//							new_item.rc.set_position(initial_inventory_root_position());
//							new_item.rc.set_size(0, 0);
//							new_item.is_container_open = true;
//						}
//						else {
//							new_item.rc.set_position(previous_slot_meta[cosmos[new_item.item].get<components::item>().current_slot].rc.get_position());
//							new_item.rc.set_size(64, 64);
//						}
//
//						if (cached_item_meta.find(new_entry.first) != cached_item_meta.end())
//							new_item.drag_offset_in_item_deposit = cached_item_meta[new_entry.first].drag_offset_in_item_deposit;
//
//						previous_item_meta.insert(new_entry);
//					}
//				}
//
//				// construct raw gui rectangle tree from metadata of items and slots 
//
//				for (auto& entry : previous_item_meta) {
//					bool is_it_root = entry.first == root;
//
//					if (!is_it_root) {
//						auto item_parent = cosmos[entry.second.item].get<components::item>().current_slot.container_entity;
//						get_meta(item_parent).children.push_back(&entry.second);
//					}
//				}
//
//				for (auto& entry : previous_slot_meta) {
//					auto parent = entry.second.slot_id.container_entity;
//					auto& meta = get_meta(parent);
//					meta.children.push_back(&entry.second);
//				}
//			}
//		}
//		
//		if (!is_gui_look_enabled)
//			return;
//
//		auto& intents = step.messages.get_queue<messages::intent_message>();
//
//		for (auto e : intents) {
//			if (e.has_state_for_gui) {
//				const auto subject = cosmos[e.subject];
//
//				subject.get<components::gui_element>();
//			}
//		}
//
//
//		auto window_inputs = step.entropy.local;
//
//		if (freeze_gui_model()) {
//			buffered_inputs_during_freeze.insert(buffered_inputs_during_freeze.end(), window_inputs.begin(), window_inputs.end());
//			return;
//		}
//
//		window_inputs.insert(window_inputs.begin(), buffered_inputs_during_freeze.begin(), buffered_inputs_during_freeze.end());
//		buffered_inputs_during_freeze.clear();
//
//		for (auto w : window_inputs)
//			gui.consume_raw_input(w);
//
//		gui.perform_logic_step();
	}

	void gui_element::draw_complete_gui_for_camera_rendering_request(viewing_step& step) const {
		//r.renderer.push_triangles(gui.draw_triangles());
		//
		//if (is_gui_look_enabled)
		//	gui.draw_cursor_and_tooltip(r);
	}
}