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
			.def("vsync", &window::glwindow::vsync)
			.def("swap_buffers", &window::glwindow::swap_buffers)
			.def("current", &window::glwindow::current)
			.enum_("border_type")[
				luabind::value("ALL_WINDOW_ELEMENTS", window::glwindow::flag::ALL_WINDOW_ELEMENTS),
				luabind::value("CAPTION", window::glwindow::flag::CAPTION),
				luabind::value("MAXIMIZE_BOX", window::glwindow::flag::MAXIMIZE_BOX),
				luabind::value("MENU", window::glwindow::flag::MENU),
				luabind::value("MINIMIZE_BOX", window::glwindow::flag::MINIMIZE_BOX),
				luabind::value("RESIZABLE", window::glwindow::flag::RESIZABLE)
			]

			);
	}
}