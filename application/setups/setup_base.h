#pragma once
#include "augs/window_framework/window.h"
#include "augs/misc/machine_entropy.h"

class setup_base {
public:
	augs::window::event::keys::key exit_key = augs::window::event::keys::ESC;
	volatile bool should_quit = false;

	bool process_exit_key(const augs::machine_entropy::local_type&);
};