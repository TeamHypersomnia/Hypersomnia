#pragma once
#include "window_framework/window.h"

namespace framework {
	extern void init();
	extern void deinit();
	extern void set_current_window(augs::window::glwindow&);
}