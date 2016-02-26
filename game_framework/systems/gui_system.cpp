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

void gui_system::rebuild_gui_tree_based_on_game_state() {
	for (auto& t : targets) {
		auto* item_slot_transfers = t->find<components::item_slot_transfers>();
		auto& element = t->get<components::gui_element>();


		// if (item_slot_transfers) {
		// 	if (element.slot_metadata.empty()) {
		// 
		// 	}
		// }
		// 
		// auto* crosshair = t->find<components::crosshair>();
		// 
		// if (crosshair) {
		// 
		// }
	}
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
