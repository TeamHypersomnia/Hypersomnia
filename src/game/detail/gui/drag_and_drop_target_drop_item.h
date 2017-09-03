#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/game_gui_context.h"

struct drag_and_drop_target_drop_item : game_gui_rect_node {
	typedef dereferenced_location<drag_and_drop_target_drop_item_in_character_gui> this_pointer;
	typedef const_dereferenced_location<drag_and_drop_target_drop_item_in_character_gui> const_this_pointer;
	typedef typename game_gui_rect_node::gui_entropy gui_entropy;

	drag_and_drop_target_drop_item();

	rgba color = red;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	static void draw(const viewing_game_gui_context context, const const_this_pointer this_id);
	static void respond_to_events(const game_gui_context, const this_pointer this_id, const gui_entropy& entropies);
	static void rebuild_layouts(const game_gui_context, const this_pointer this_id);
};