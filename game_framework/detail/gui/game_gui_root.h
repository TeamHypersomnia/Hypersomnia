#pragma once
#include "special_drag_and_drop_target.h"
#include "gui/gui_world.h"
#include "drag_and_drop.h"

struct game_gui_root : public augs::gui::rect {
	augs::gui::rect parent_of_inventory_controls;
	augs::gui::rect parent_of_game_windows;
	special_drag_and_drop_target drop_item_icon = special_drag_and_drop_target(augs::gui::material(assets::texture_id::DROP_HAND_ICON, red));

	game_gui_root();
	void get_member_children(std::vector<augs::gui::rect_id>& children) final;
};

class gui_system;
struct game_gui_world : public augs::gui::gui_world {
	gui_system* gui_system = nullptr;

	drag_and_drop_result prepare_drag_and_drop_result();
};