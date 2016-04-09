#pragma once
#include "game_framework/messages/raw_window_input_message.h"
#include "game_framework/messages/camera_render_request_message.h"
#include "special_drag_and_drop_target.h"
#include "gui/text_drawer.h"
#include "gui/gui_world.h"
#include "drag_and_drop.h"
#include "aabb_highlighter.h"

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

	void consume_raw_input(messages::raw_window_input_message&);
	void draw_cursor_and_tooltip(messages::camera_render_request_message);

	augs::entity_id get_hovered_world_entity(vec2 camera_pos);
	drag_and_drop_result prepare_drag_and_drop_result();
};