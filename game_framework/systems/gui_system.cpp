#include "gui_system.h"
#include "graphics/renderer.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"
#include "../messages/raw_window_input_message.h"

#include "../components/item_component.h"
#include "../components/input_receiver_component.h"
#include "../systems/input_system.h"

#include "crosshair_system.h"
#include "game_framework/settings.h"

gui_system::gui_system(world& parent_world) : processing_system_templated(parent_world) {
	gui.gui_system = this;
	gui.root.children.push_back(&game_gui_root);
	gui.root.clip = false;
}

bool gui_system::freeze_gui_model() {
	return parent_world.get_system<input_system>().gui_item_transfer_intent_player.get_pending_inputs_for_logic().size() > 0;
}

void gui_system::draw_gui_overlays_for_camera_rendering_request(messages::camera_render_request_message r) {
	gui.draw_triangles();
	r.state.output->push_triangles_from_gui_world(gui);

	if (is_gui_look_enabled)
		gui.draw_cursor_and_tooltip(r);
}

augs::entity_id gui_system::get_game_world_crosshair() {
	auto& crosshairs = parent_world.get_system<crosshair_system>().targets;

	for (auto& c : crosshairs) {
		if (c->is_enabled<components::input_receiver>()) {
			return c;
		}
	}
}

void gui_system::translate_raw_window_inputs_to_gui_events() {
	if (!is_gui_look_enabled)
		return;
	
	auto window_inputs = parent_world.get_message_queue<messages::raw_window_input_message>();

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

void gui_system::suppress_inputs_meant_for_gui() {
	if (!is_gui_look_enabled)
		return;
	
	auto& inputs = parent_world.get_message_queue<messages::raw_window_input_message>();

	for (auto& it : inputs) {
		if (it.raw_window_input.msg != window::event::keydown &&
			it.raw_window_input.msg != window::event::keyup) {
			it.delete_this_message = true;
		}
	}

	parent_world.delete_marked_messages(inputs);
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