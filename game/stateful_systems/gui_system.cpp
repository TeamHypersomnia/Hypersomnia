#include "gui_system.h"
#include "graphics/renderer.h"

#include "game/entity_id.h"
#include "game/cosmos.h"

#include "game/messages/intent_message.h"
#include "game/messages/unmapped_intent_message.h"

#include "game/components/item_component.h"
#include "game/components/input_receiver_component.h"
#include "game/systems/input_system.h"

#include "game/systems/crosshair_system.h"
#include "game/entity_handle.h"

gui_system::gui_system() {
	gui.gui_system = this;
	gui.root.children.push_back(&game_gui_root);
	gui.root.clip = false;
}

bool gui_system::freeze_gui_model() {
	return false;
	//return parent_cosmos.temporary_systems.get<input_system>().gui_item_transfer_intent_player.get_pending_inputs_for_logic().size() > 0;
}

void gui_system::draw_complete_gui_for_camera_rendering_request(viewing_step& r) const {
	r.renderer.push_triangles(gui.draw_triangles());

	if (is_gui_look_enabled)
		gui.draw_cursor_and_tooltip(r);
}

entity_id gui_system::get_cosmos_crosshair(const cosmos& cosm) {
	for (auto it : cosm.get(processing_subjects::WITH_CROSSHAIR)) {
		if (it.get<components::processing>().is_in(processing_subjects::WITH_INPUT_RECEIVER)) {
			return it;
		}
	}
}

void gui_system::translate_raw_window_inputs_to_gui_events(augs::machine_entropy entropy) {
	if (!is_gui_look_enabled)
		return;
	
	auto window_inputs = entropy.local;

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

void gui_system::suppress_inputs_meant_for_gui(augs::machine_entropy& entropy) {
	if (!is_gui_look_enabled)
		return;
	
	erase_remove(entropy.local, [](auto it) {
		if (it.msg != window::event::keydown &&
			it.msg != window::event::keyup) {
			return true;
		}

		return false;
	});
}

void gui_system::switch_to_gui_mode_and_back(fixed_step& step) {
	auto& intents = step.messages.get_queue<messages::unmapped_intent_message>();

	for (auto& i : intents) {
		if (i.intent == intent_type::SWITCH_TO_GUI && i.pressed_flag) {
			is_gui_look_enabled = !is_gui_look_enabled;
		}

		if (i.intent == intent_type::START_PICKING_UP_ITEMS) {
			//preview_due_to_item_picking_request = i.pressed_flag;
		}
	}
}

void gui_system::translate_game_events_for_hud(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	hud.acquire_game_events(step);
}