#pragma once
#include "game/step.h"
#include "special_drag_and_drop_target.h"
#include "gui/text_drawer.h"
#include "gui/gui_world.h"
#include "drag_and_drop.h"
#include "aabb_highlighter.h"

#include "misc/machine_entropy.h"

class fixed_step;

struct game_gui_root : public augs::gui::rect {
	augs::gui::rect parent_of_inventory_controls;
	augs::gui::rect parent_of_game_windows;
	special_drag_and_drop_target drop_item_icon = special_drag_and_drop_target(augs::gui::material(assets::texture_id::DROP_HAND_ICON, red));

	game_gui_root();
	void get_member_children(std::vector<augs::gui::rect_id>& children) const final;
};

class gui_system;
struct game_gui_world : public augs::gui::gui_world {
	vec2 gui_crosshair_position;

	aabb_highlighter world_hover_highlighter;

	augs::gui::text_drawer tooltip_drawer;
	augs::gui::text_drawer description_drawer;
	augs::gui::text_drawer dragged_charges_drawer;
	int dragged_charges = 0;

	vec2i size;

	void resize(vec2i size) {
		this->size = size;
	}

	void consume_raw_input(augs::window::event::state&);
	void draw_cursor_and_tooltip(viewing_step&) const;

	entity_id get_hovered_world_entity(vec2 camera_pos);
	drag_and_drop_result prepare_drag_and_drop_result() const;
};