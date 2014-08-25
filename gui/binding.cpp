#pragma once
#include "stdafx.h"
#include "utilities/lua_state_wrapper.h"
#include "hypersomnia_gui.h"

void hypersomnia_gui::bind(augs::lua_state_wrapper& wrapper) {
	luabind::module(wrapper.raw)[
		luabind::class_<hypersomnia_gui>("hypersomnia_gui")
		.def(luabind::constructor<augs::window::glwindow&>())
		.def("poll_events", &poll_events)
		.def("draw_call", &draw_call)
	];
}