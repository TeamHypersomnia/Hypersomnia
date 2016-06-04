#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "window_framework/window.h"
#include "window_framework/platform_utils.h"
#include "../bind_game_and_augs.h"

namespace bindings {
	luabind::scope _glwindow() {
		return
			(
			luabind::def("get_display", window::get_display),
			luabind::def("set_display", window::set_display),
			
			luabind::def("set_cursor_visible", window::set_cursor_visible),
			luabind::def("colored_print", augs::colored_print),
			
			luabind::class_<window::glwindow>("glwindow")
			.def(luabind::constructor<>())
			.def("create", &window::glwindow::create)
			.def("set_vsync", &window::glwindow::set_vsync)
			.def("swap_buffers", &window::glwindow::swap_buffers)
			.def("set_as_current", &window::glwindow::set_as_current)
			);
	}
}