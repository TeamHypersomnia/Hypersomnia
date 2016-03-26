#include "game_gui_root.h"

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