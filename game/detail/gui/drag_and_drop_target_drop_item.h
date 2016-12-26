#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/gui_context.h"

struct drag_and_drop_target_drop_item : game_gui_rect_node {
	typedef drag_and_drop_target_drop_item_location location;
	typedef dereferenced_location<drag_and_drop_target_drop_item> this_pointer;
	typedef dereferenced_location<const drag_and_drop_target_drop_item> const_this_pointer;

	drag_and_drop_target_drop_item(const augs::gui::material new_mat);

	augs::gui::material mat;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	static void draw(const viewing_gui_context& context, const const_this_pointer& this_id, augs::gui::draw_info info);
	static void consume_gui_event(const logic_gui_context& context, const this_pointer& this_id, const augs::gui::event_info info);
	static void perform_logic_step(const logic_gui_context&, const this_pointer& this_id);
};