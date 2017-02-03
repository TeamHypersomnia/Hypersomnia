#pragma once
#include "augs/gui/rect.h"
#include "augs/gui/material.h"
#include "augs/gui/appearance_detector.h"

#include "game/detail/gui/game_gui_context.h"

struct sentience_meter : game_gui_rect_node {
	typedef dereferenced_location<sentience_meter_in_character_gui> this_pointer;
	typedef const_dereferenced_location<sentience_meter_in_character_gui> const_this_pointer;
	typedef typename game_gui_rect_node::gui_entropy gui_entropy;

	augs::gui::appearance_detector detector;

	sentience_meter();

	augs::gui::material get_icon_mat(const const_this_pointer this_id) const;

	static void draw(
		const viewing_game_gui_context context, 
		const const_this_pointer this_id, 
		augs::gui::draw_info info
	);

	static void advance_elements(
		const game_gui_context, 
		const this_pointer this_id, 
		const gui_entropy& entropies, 
		const augs::delta
	);

	static void rebuild_layouts(
		const game_gui_context, 
		const this_pointer this_id
	);
};