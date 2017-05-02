#pragma once
#include "augs/window_framework/event.h"
#include "augs/misc/machine_entropy.h"

class config_lua_table;
class viewing_session;

class setup_base {
public:
	augs::window::event::keys::key exit_key = augs::window::event::keys::key::ESC;
	volatile bool should_quit = false;

	bool process_exit_key(const augs::machine_entropy::local_type&);
};