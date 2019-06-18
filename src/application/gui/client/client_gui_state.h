#pragma once
#include "application/gui/client/rcon_gui.h"
#include "application/gui/client/chat_gui.h"
#include "application/setups/setup_common.h"

struct client_gui_state {
	chat_gui_state chat;
	rcon_gui_state rcon;

	bool control(const handle_input_before_game_input in);
};
