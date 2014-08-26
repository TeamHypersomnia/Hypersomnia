#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "graphics/text/printer.h"
#include "graphics/text/drafter.h"

#include "misc/vector_wrapper.h"

using namespace graphics::gui::text;

wchar_t towchar(const std::wstring& s) {
	return s[0];
}

std::string wchar_vec_to_str(misc::vector_wrapper<wchar_t> str) {
	auto out_w = std::wstring(str.raw.begin(), str.raw.end());

	return std::string(out_w.begin(), out_w.end());
}

namespace bindings {
	luabind::scope _text() {
		return

			luabind::class_<wchar_t>("wchar_t"),

			luabind::class_<formatted_char>("formatted_char")
			.def(luabind::constructor<>())
			.def_readwrite("font_used", &formatted_char::font_used)
			.def_readwrite("c", &formatted_char::c)
			.def_readwrite("r", &formatted_char::r)
			.def_readwrite("g", &formatted_char::g)
			.def_readwrite("b", &formatted_char::b)
			.def_readwrite("a", &formatted_char::a),

			misc::vector_wrapper<wchar_t>::bind("wchar_t_vec"),
			misc::vector_wrapper<formatted_char>::bind_vector("formatted_text"),
			
			luabind::def("wchar_vec_to_str", wchar_vec_to_str),
			luabind::def("towchar", towchar),
			luabind::def("towchar_vec", towchar_vec),
			luabind::def("get_text_bbox", get_text_bbox),
			luabind::def("quick_print_text", quick_print)
			
			//.def("append", &graphics::gui::text::fstr::append)

			//,


			//.def(luabind::constructor<>())
			//.def_readwrite("camera_transform", &renderable::draw_input::camera_transform)
			//.def_readwrite("visible_area", &renderable::draw_input::visible_area)
			//.def_readwrite("additional_info", &renderable::draw_input::additional_info)
			//.def_readwrite("output", &renderable::draw_input::output)
			//.def_readwrite("transform", &renderable::draw_input::transform)
			//,

			;
	}
}