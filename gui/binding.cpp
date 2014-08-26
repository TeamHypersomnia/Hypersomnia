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

void callback_textbox::append_text(augs::misc::vector_wrapper<wchar_t>& wstr, augs::graphics::pixel_32 color) {
	auto prev_caret_pos = textbox_object.editor.get_caret_pos();
	auto prev_caret_sel = textbox_object.editor.get_selection_offset();

	textbox_object.editor.set_caret_end(false);

	auto resultant_style = textbox_object.editor.get_default_style();
	resultant_style.color = pixel_32(color.r, color.g, color.b, color.a);

	textbox_object.editor.insert(format(std::wstring(wstr.raw.begin(), wstr.raw.end()), resultant_style));

	textbox_object.editor.set_caret(prev_caret_pos, false);
	textbox_object.editor.set_selection_offset(prev_caret_sel);
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
		.def("append_text", &callback_textbox::append_text)
		.def("setup", &callback_textbox::setup)
		.def("set_command_callback", &callback_textbox::set_command_callback)
	];
}