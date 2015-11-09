#pragma once
#include "window_framework/window.h"
#include "augmentations.h"
namespace augs {
	struct lua_state_wrapper;
}

namespace framework {
	extern void init(unsigned which = augs::which_augs::ALL);
	extern void deinit();
	extern void run_tests();
	extern void set_current_window(augs::window::glwindow&);
	extern void bind_whole_engine(augs::lua_state_wrapper&, std::function<void()> custom_world_binding = std::function<void()>());
}