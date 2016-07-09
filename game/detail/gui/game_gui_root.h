#pragma once
#include "game/step.h"
#include "special_drag_and_drop_target.h"
#include "gui/text_drawer.h"
#include "gui/gui_world.h"
#include "drag_and_drop.h"
#include "aabb_highlighter.h"

#include "game/machine_entropy.h"

class fixed_step;

struct game_gui_root : public augs::gui::rect {
	augs::gui::rect parent_of_inventory_controls;

	game_gui_root();
	void get_member_children(std::vector<augs::gui::rect_id>& children) const final;
};

class gui_system;
struct game_gui_world : public augs::gui::gui_world {
	void consume_raw_input(augs::window::event::state&);
	void draw_cursor_and_tooltip(viewing_step&) const;

	entity_id get_hovered_world_entity(vec2 camera_pos);
	drag_and_drop_result prepare_drag_and_drop_result() const;
};