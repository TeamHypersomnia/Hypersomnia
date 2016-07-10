#pragma once
#include <functional>
#include "game/entity_handle_declaration.h"

class fixed_step;
class viewing_step;
class game_gui_world;

class gui_system {
	template<bool is_const>
	void tree_callback(basic_entity_handle<is_const>, std::function<void()>);

public:
	void translate_game_events_for_hud(fixed_step&);

	void advance_gui_elements(fixed_step&);
	
	void switch_to_gui_mode_and_back(fixed_step&);	
};