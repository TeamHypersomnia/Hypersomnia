#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "view/game_gui/game_gui_context.h"

struct drag_and_drop_target_drop_item : game_gui_rect_node {
	using this_pointer = dereferenced_location<drag_and_drop_target_drop_item_in_character_gui> ;
	using const_this_pointer = const_dereferenced_location<drag_and_drop_target_drop_item_in_character_gui>;
	using gui_entropy = typename game_gui_rect_node::gui_entropy;

	drag_and_drop_target_drop_item();

	rgba color = red;

	vec2i user_drag_offset;
	vec2i initial_pos;

	augs::gui::appearance_detector detector;

	static void draw(const viewing_game_gui_context context, const const_this_pointer this_id);
	static void respond_to_events(const game_gui_context, const this_pointer this_id, const gui_entropy& entropies);
	static void rebuild_layouts(const game_gui_context, const this_pointer this_id);
};