#pragma once
#include "application/setups/client/rcon_pane.h"

struct rcon_gui_state {
	rcon_pane active_pane = rcon_pane::ARENAS;
	bool show = false;
};
