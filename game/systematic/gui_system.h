#pragma once
#include <functional>
#include "game/transcendental/entity_handle_declaration.h"

class fixed_step;
class viewing_step;
class game_gui_world;

class gui_system {
public:
	void translate_game_events_for_hud(fixed_step&);
	void advance_gui_elements(fixed_step&);
	void switch_to_gui_mode_and_back(fixed_step&);	
};