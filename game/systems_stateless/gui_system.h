#pragma once
#include <functional>
#include "game/transcendental/entity_handle_declaration.h"

class logic_step;
class viewing_step;
class game_gui_world;

class gui_system {
public:
	void advance_gui_elements(logic_step&);
	void switch_to_gui_mode_and_back(logic_step&);	
};