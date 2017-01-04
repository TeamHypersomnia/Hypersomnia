#pragma once
#include "augs/window_framework/event.h"
#include "augs/misc/machine_entropy.h"
#include "application/config_lua_table.h"

class setup_base {
public:
	augs::window::event::keys::key exit_key = augs::window::event::keys::key::ESC;
	volatile bool should_quit = false;

	bool process_exit_key(const augs::machine_entropy::local_type&);
};