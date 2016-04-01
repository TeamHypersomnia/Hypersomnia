#include "game_gui_root.h"
#include "game_framework/systems/gui_system.h"
#include "item_button.h"
#include "entity_system/world.h"

void game_gui_root::get_member_children(std::vector<augs::gui::rect_id>& children) {
	children.push_back(&parent_of_inventory_controls);
	children.push_back(&drop_item_icon);
	children.push_back(&parent_of_game_windows);
}

game_gui_root::game_gui_root() {
	clip = false;
	parent_of_inventory_controls.clip = false;
	parent_of_game_windows.clip = false;
}

void game_gui_world::consume_raw_input(messages::raw_window_input_message& w) {
	if (w.raw_window_input.msg == window::event::mousemotion) {
		gui_crosshair_position += w.raw_window_input.mouse.rel;
		gui_crosshair_position.clamp_from_zero_to(vec2(size.x - 1, size.y - 1));
	}

	w.raw_window_input.mouse.pos = gui_crosshair_position;

	bool fetched = false;

	if (w.raw_window_input.msg == window::event::rdown) {
		auto* dragged_item = dynamic_cast<item_button*>(rect_held_by_lmb);

		if (dragged_item && dragged_item->is_being_dragged(*this)) {
			messages::gui_item_transfer_intent intent;
			intent.item = dragged_item->item;
			intent.target_slot.unset();
			gui_system->parent_world.post_message(intent);
			fetched = true;
		}
	}

	if (w.raw_window_input.msg == window::event::wheel) {

	}

	if (!fetched)
		consume_raw_input_and_generate_gui_events(w.raw_window_input);
}
