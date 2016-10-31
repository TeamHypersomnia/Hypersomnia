#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/dispatcher_context.h"

struct drag_and_drop_target_drop_item : game_gui_rect_node {
	typedef drag_and_drop_target_drop_item_location location;
	typedef location_and_pointer<drag_and_drop_target_drop_item> this_pointer;
	typedef location_and_pointer<const drag_and_drop_target_drop_item> const_this_pointer;

	drag_and_drop_target_drop_item(const augs::gui::material new_mat);

	augs::gui::material mat;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	static void draw(const viewing_dispatcher_context& context, const const_this_pointer& this_id, augs::gui::draw_info info);
	static void consume_gui_event(const dispatcher_context& context, const this_pointer& this_id, const augs::gui::event_info info);
	static void perform_logic_step(const dispatcher_context&, const this_pointer& this_id);
};