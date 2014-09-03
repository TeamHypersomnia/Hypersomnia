#pragma once
#include "stdafx.h"
#include "utilities/lua_state_wrapper.h"
#include "hypersomnia_gui.h"
#include "misc/vector_wrapper.h"

#include "misc/stream.h"
#include <chrono>
#include <ctime>
void callback_textbox::set_command_callback(luabind::object callback) {
	//luabind::object callbackobj = *callback;
	textbox_object.command_callback = [callback](std::wstring& wstr)	{
		luabind::call_function<void>(callback, augs::misc::towchar_vec(wstr));
	};
}

augs::misc::vector_wrapper<wchar_t> get_local_time() {
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t(now);
	auto tm = std::localtime(&now_c);

	auto hours = augs::misc::wstr<int>(tm->tm_hour);
	auto mins = augs::misc::wstr<int>(tm->tm_min);
	auto secs = augs::misc::wstr<int>(tm->tm_sec);

	if (tm->tm_hour < 10) hours = L'0' + hours;
	if (tm->tm_min < 10) mins = L'0' + mins;
	if (tm->tm_sec < 10) secs = L'0' + secs;

	return augs::misc::towchar_vec
		(L'[' + hours + L':' + mins + L':' + secs + L']');
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

void callback_textbox::append_text(augs::graphics::gui::text::fstr& str, bool set_default_font) {
	auto prev_caret_pos = textbox_object.editor.get_caret_pos();
	auto prev_caret_sel = textbox_object.editor.get_selection_offset();

	textbox_object.editor.set_caret_end(false);

	if (set_default_font) {
		auto def_style = textbox_object.editor.get_default_style();
	
		for (auto& c : str) {
			c.font_used = def_style.f;
		}
	}

	textbox_object.editor.insert(str);

	textbox_object.editor.set_caret(prev_caret_pos, false);
	textbox_object.editor.set_selection_offset(prev_caret_sel);
}
using namespace graphics::gui;

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

	if (s) 
		s->color.set(rgba);
}

void set_border(stylesheet& subject, std::string attr, int w, augs::graphics::pixel_32 rgba) {
	auto* s = resolve_attr(subject, attr);

	if (s) 
		s->border.set(solid_stroke(w, material(rgba)));
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
		.def("blur", &blur)
		.def("draw_call", &draw_call),

		luabind::class_<callback_textbox>("callback_textbox")
		.def(luabind::constructor<hypersomnia_gui&>())
		.def("append_text", &callback_textbox::append_text)
		.def("setup", &callback_textbox::setup)
		.def("set_caret", &callback_textbox::set_caret)
		.def("backspace", &callback_textbox::backspace)
		.def("view_caret", &callback_textbox::view_caret)
		.def("remove_line", &callback_textbox::remove_line)
		.def("is_focused", &callback_textbox::is_focused)
		.def("focus", &callback_textbox::focus)
		.def("get_length", &callback_textbox::get_length)
		.def("draw", &callback_textbox::draw)
		.def("clear_text", &callback_textbox::clear_text)
		.def("is_clean", &callback_textbox::is_clean)
		.def("set_area", &callback_textbox::set_area)
		.def("get_text_bbox", &callback_textbox::get_text_bbox)
		.def("set_alpha_range", &callback_textbox::set_alpha_range)
		.def("set_command_callback", &callback_textbox::set_command_callback),

		luabind::def("get_local_time", get_local_time),

		luabind::def("set_color", wrap_set(set_color, get_textbox_style, callback_textbox)),
		luabind::def("set_color", wrap_set(set_color, get_rect_style, callback_rect)),
		
		luabind::def("set_border", wrap_set(set_border, get_textbox_style, callback_textbox)),
		luabind::def("set_border", wrap_set(set_border, get_rect_style, callback_rect)),

		luabind::class_<callback_rect>("callback_rect")
		.def(luabind::constructor<hypersomnia_gui&>())
		.def("setup", &callback_rect::setup)
		.def("focus", &callback_rect::focus)
		.def("set_focus_callback", &callback_rect::set_focus_callback)
		.def("set_lpressed_callback", &callback_rect::set_lpressed_callback)
		.def("set_hover_callback", &callback_rect::set_hover_callback)
		.def("set_blur_callback", &callback_rect::set_blur_callback)
	];
}