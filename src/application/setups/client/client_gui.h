#pragma once
#include "view/mode_gui/arena/arena_mode_gui.h"

struct spectator_gui_state {
	mode_player_id now_spectating;
};

struct client_gui_state {
	arena_gui_state arena_gui;
	spectator_gui_state spectator_gui;

	client_gui_state() {
		arena_gui.choose_team.show = true;
	}
};
