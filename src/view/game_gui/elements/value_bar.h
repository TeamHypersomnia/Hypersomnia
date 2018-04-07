#pragma once
#include "augs/templates/type_list.h"

#include "augs/gui/rect.h"
#include "augs/gui/appearance_detector.h"

#include "view/game_gui/game_gui_context.h"

struct value_bar : game_gui_rect_node {
	typedef dereferenced_location<value_bar_in_character_gui> this_pointer;
	typedef const_dereferenced_location<value_bar_in_character_gui> const_this_pointer;
	typedef typename game_gui_rect_node::gui_entropy gui_entropy;

	augs::gui::appearance_detector detector;
	
	struct effect_particle {
		vec2i relative_pos;
		assets::necessary_image_id tex;
	};

	border_input border = { 1, 1 };
	
	float seconds_accumulated = 0.f;

	std::vector<effect_particle> particles;

	value_bar();

	static assets::image_id get_bar_icon(
		const const_game_gui_context context, 
		const const_this_pointer this_id
	);

	static rgba get_bar_col(
		const const_game_gui_context context, 
		const const_this_pointer this_id
	);

	static bool is_sentience_meter(const const_this_pointer);
	static bool is_enabled(const const_game_gui_context context, const unsigned vertical_index);

	static std::string get_description_for_hover(
		const const_game_gui_context context,
		const const_this_pointer
	);

	static ltrb get_value_bar_rect(
		const const_game_gui_context context,
		const const_this_pointer this_id,
		const ltrb absolute
	);

	static ltrb get_bar_rect_with_borders(
		const const_game_gui_context context,
		const const_this_pointer this_id,
		const ltrb absolute
	);

	static void draw(
		const viewing_game_gui_context context, 
		const const_this_pointer this_id
	);

	static void advance_elements(
		const game_gui_context, 
		const this_pointer this_id, 
		const augs::delta
	);

	static void respond_to_events(
		const game_gui_context,
		const this_pointer this_id,
		const gui_entropy& entropies
	);

	static void rebuild_layouts(
		const game_gui_context, 
		const this_pointer this_id
	);
};