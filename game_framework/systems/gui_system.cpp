#include "gui_system.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"
#include "../messages/raw_window_input_message.h"

#include "crosshair_system.h"

#include "graphics/renderer.h"

#include "../components/sprite_component.h"
#include "../components/input_receiver_component.h"

#include "../assets/texture.h"
#include "../shared/inventory_slot.h"

#include "gui/stroke.h"

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

void slot_rect::draw_triangles(draw_info info) {
	auto is_hand_slot = slot_id.is_hand_slot();

	rgba inside_attachment_col = orange;
	inside_attachment_col.a = 12;

	rgba attachment_border_col = violet;
	attachment_border_col.a = 16;

	rgba inside_deposit_col = cyan;
	inside_attachment_col.a = 12;

	rgba deposit_border_col = cyan;
	attachment_border_col.a = 255;

	if (is_hand_slot) {
		inside_attachment_col = cyan;
		inside_attachment_col.a = 12;
		attachment_border_col = cyan;
		attachment_border_col.a = 255;
	}

	augs::gui::material inside_deposit_mat(assets::texture_id::BLANK, inside_deposit_col);
	augs::gui::material inside_attachment_mat(assets::texture_id::BLANK, inside_attachment_col);
	augs::gui::material attachment_border_mat(assets::texture_id::BLANK, attachment_border_col);
	augs::gui::material deposit_border_mat(assets::texture_id::BLANK, deposit_border_col);

	if (slot_id->is_attachment_slot) {
		if (slot_id.has_items()) {
			return;
		}
		else {
			draw_rectangle_with_material(info, inside_attachment_mat);

			augs::gui::solid_stroke border(1, attachment_border_mat);
			border.draw(info.v, *this);
		}
	}
	else {

	}
}

void slot_rect::consume_gui_event(event_info info) {
	detector.update_appearance(info);
}

void gui_system::rebuild_gui_tree_based_on_game_state() {
	std::vector<augs::gui::rect_id> new_inventory_elements;

	for (auto& t : targets) {
		auto* item_slot_transfers = t->find<components::item_slot_transfers>();
		auto& element = t->get<components::gui_element>();

		if (item_slot_transfers) {
			auto& slot_meta = element.slot_metadata;
			auto& item_meta = element.item_metadata;

			/* create new if not found */

			if (slot_meta.find(t[slot_function::PRIMARY_HAND]) == slot_meta.end()) {
				slot_rect primary_hand_rect;
				primary_hand_rect.slot_id = t[slot_function::PRIMARY_HAND];
				primary_hand_rect.rc = rects::xywh<float>(size.x - 100, size.y - 100, 32, 32);

				slot_meta[t[slot_function::PRIMARY_HAND]] = primary_hand_rect;
			}


			for (auto& entry : slot_meta)
				new_inventory_elements.push_back(&entry.second);

			for (auto& entry : item_meta)
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

			w.raw_window_input.mouse.pos = gui_crosshair_position;
		}

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
