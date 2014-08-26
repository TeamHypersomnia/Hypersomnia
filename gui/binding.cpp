#pragma once
#include "stdafx.h"
#include "utilities/lua_state_wrapper.h"
#include "hypersomnia_gui.h"
#include "misc/vector_wrapper.h"

void callback_textbox::set_command_callback(luabind::object callback) {
	//luabind::object callbackobj = *callback;
	textbox_object.command_callback = [callback](std::wstring& wstr)	{
		luabind::call_function<void>(callback, augs::misc::towchar_vec(wstr));
	};
}

void hypersomnia_gui::bind(augs::lua_state_wrapper& wrapper) {
	luabind::module(wrapper.raw)[
		luabind::class_<hypersomnia_gui>("hypersomnia_gui")
		.def(luabind::constructor<augs::window::glwindow&>())
		.def("poll_events", &poll_events)
		.def("setup", &setup)
		.def("draw_call", &draw_call),

		luabind::class_<callback_textbox>("callback_textbox")
		.def(luabind::constructor<hypersomnia_gui&>())
		.def("setup", &callback_textbox::setup)
		.def("set_command_callback", &callback_textbox::set_command_callback)
	];
}