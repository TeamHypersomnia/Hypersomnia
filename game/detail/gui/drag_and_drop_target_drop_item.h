#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/game_gui_context.h"

struct drag_and_drop_target_drop_item : game_gui_rect_node {
	typedef dereferenced_location<drag_and_drop_target_drop_item_in_gui_element> this_pointer;
	typedef const_dereferenced_location<drag_and_drop_target_drop_item_in_gui_element> const_this_pointer;
	typedef typename game_gui_rect_node::gui_entropy gui_entropy;

	drag_and_drop_target_drop_item(const augs::gui::material new_mat);

	augs::gui::material mat;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	static void draw(const viewing_gui_context context, const const_this_pointer this_id, augs::gui::draw_info info);
	static void advance_elements(const logic_gui_context, const this_pointer this_id, const gui_entropy& entropies, const augs::delta);
	static void rebuild_layouts(const logic_gui_context, const this_pointer this_id);
};