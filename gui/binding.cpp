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


void callback_rect::set_focus_callback(luabind::object callback) {
	rect_obj.focus_callback = [callback]()	{
		luabind::call_function<void>(callback);
	};
}

void callback_rect::set_lpressed_callback(luabind::object callback) {
	rect_obj.lpressed_callback = [callback]()	{
		luabind::call_function<void>(callback);
	};
}

void callback_rect::set_hover_callback(luabind::object callback) {
	rect_obj.hover_callback = [callback]()	{
		luabind::call_function<void>(callback);
	};
}

void callback_rect::set_blur_callback(luabind::object callback) {
	rect_obj.blur_callback = [callback]()	{
		luabind::call_function<void>(callback);
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

stylesheet::style* resolve_attr(stylesheet& subject, std::string a) {
	if (a == "released") {
		return &subject.released;
	}
	else if (a == "focused") {
		return &subject.focused;
	}
	else if (a == "hovered") {
		return &subject.hovered;
	}
	else if (a == "pushed") {
		return &subject.pushed;
	}

	return nullptr;
}

void set_color(stylesheet& subject, std::string attr, augs::graphics::pixel_32 rgba) {
	auto* s = resolve_attr(subject, attr);

	if (s) {
		db::graphics::pixel_32 col;
		memcpy(&col, &rgba, sizeof(col));
		s->color.set(col);
	}
}

void set_border(stylesheet& subject, std::string attr, int w, augs::graphics::pixel_32 rgba) {
	auto* s = resolve_attr(subject, attr);

	if (s) {
		db::graphics::pixel_32 col;
		memcpy(&col, &rgba, sizeof(col));

		s->border.set(solid_stroke(w, material(col)));
	}
}


template <typename... Args>
void invokem (Args...) {

}


template<typename Signature>
struct setter_wrapper;

template<typename Ret, typename First, typename... Args>
struct setter_wrapper<Ret (First, Args...)> {
	template <typename T, stylesheet& get_style(T&), Ret setter_func(First, Args...)>
	static Ret setter(T& subject, Args... arguments) {
		return setter_func(get_style(subject), arguments...);
	}
};

#define wrap_set(_s, _g, _t) setter_wrapper<decltype(_s)>::setter<_t, _g, _s> 

stylesheet& get_textbox_style(callback_textbox& c) {
	return c.textbox_object.styles;
}

stylesheet& get_rect_style(callback_rect& c) {
	return c.rect_obj.styles;
}

void hypersomnia_gui::bind(augs::lua_state_wrapper& wrapper) {
	callback_textbox a;
	invokem(a.textbox_object.styles, "aaa", augs::graphics::pixel_32(1, 1, 1, 1));

	//setter_wrapper<decltype(set_color)>::invoke(a, "aaa", augs::graphics::pixel_32(1, 1, 1, 1));
	(a, "aaa", augs::graphics::pixel_32(1, 1, 1, 1));
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
		.def("set_command_callback", &callback_textbox::set_command_callback),

		luabind::def("set_color", wrap_set(set_color, get_textbox_style, callback_textbox)),
		luabind::def("set_color", wrap_set(set_color, get_rect_style, callback_rect)),

		luabind::def("set_border", wrap_set(set_border, get_textbox_style, callback_textbox)),
		luabind::def("set_border", wrap_set(set_border, get_rect_style, callback_rect)),

		luabind::class_<callback_rect>("callback_rect")
		.def(luabind::constructor<hypersomnia_gui&>())
		.def("setup", &callback_rect::setup)
		.def("set_focus_callback", &callback_rect::set_focus_callback)
		.def("set_lpressed_callback", &callback_rect::set_lpressed_callback)
		.def("set_hover_callback", &callback_rect::set_hover_callback)
		.def("set_blur_callback", &callback_rect::set_blur_callback)

	];
}