#pragma once
#include "application/gui/client/rcon_gui.h"

struct client_gui_state {
	chat_gui_state chat;
	rcon_gui_state rcon;
};
