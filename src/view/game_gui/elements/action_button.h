#pragma once
#include "augs/math/vec2.h"
#include "augs/gui/appearance_detector.h"
#include "augs/audio/sound_source.h"

#include "game/detail/spells/all_spells.h"

#include "view/game_gui/game_gui_context.h"

class action_button : public game_gui_rect_node {
public:
	typedef game_gui_rect_node base;
	typedef base::gui_entropy gui_entropy;

	typedef dereferenced_location<action_button_in_character_gui> this_in_item;
	typedef const_dereferenced_location<action_button_in_character_gui> const_this_in_item;

	augs::gui::appearance_detector detector;

	augs::sound_source hover_sound;
	augs::sound_source click_sound;

	spell_id bound_spell;
	
	static spell_id get_bound_spell(
		const const_game_gui_context, 
		const const_this_in_item this_id
	);

	float elapsed_hover_time_ms = 0.f;

	float hover_highlight_maximum_distance = 8.f;
	float hover_highlight_duration_ms = 400.f;

	static void draw(
		const viewing_game_gui_context, 
		const const_this_in_item this_id
	);

	static void respond_to_events(
		const game_gui_context, 
		const this_in_item this_id, 
		const gui_entropy& entropies
	);

	static void advance_elements(
		const game_gui_context, 
		const this_in_item this_id, 
		const augs::delta
	);

	static void rebuild_layouts(
		const game_gui_context, 
		const this_in_item this_id
	);
};