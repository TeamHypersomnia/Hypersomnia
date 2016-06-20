#include "game_gui_root.h"
#include "game/systems/gui_system.h"
#include "game/components/item_component.h"
#include "item_button.h"
#include "game/cosmos.h"

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
	
	auto* dragged_item = dynamic_cast<item_button*>(rect_held_by_lmb);

	if (w.raw_window_input.msg == window::event::rdown
		|| w.raw_window_input.msg == window::event::rdoubleclick
		) {
		if (dragged_item && dragged_item->is_being_dragged(*this)) {
			messages::gui_item_transfer_intent intent;
			intent.item = dragged_item->item;
			intent.target_slot.unset();
			intent.specified_quantity = dragged_charges;
			gui_system->step.messages.post(intent);
			fetched = true;
		}
	}

	if (w.raw_window_input.msg == window::event::wheel) {
		if (dragged_item) {
			auto& item = dragged_item->item.get<components::item>();

			auto delta = w.raw_window_input.mouse.scroll;

			dragged_charges += delta;

			if (dragged_charges <= 0)
				dragged_charges = item.charges + dragged_charges;
			if (dragged_charges > item.charges)
				dragged_charges = dragged_charges - item.charges;

		}
		
	}

	if (!fetched)
		consume_raw_input_and_generate_gui_events(w.raw_window_input);
}
