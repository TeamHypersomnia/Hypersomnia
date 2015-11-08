#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "depthbase/gui/text/printer.h"
#include "depthbase/gui/text/drafter.h"

#include "game_framework/resources/render_info.h"
#include "game_framework/systems/render_system.h"

#include "misc/vector_wrapper.h"

using namespace graphics::gui::text;

wchar_t towchar(const std::wstring& s) {
	return s[0];
}

std::string wchar_vec_to_str(vector_wrapper<wchar_t> str) {
	auto out_w = std::wstring(str.raw.begin(), str.raw.end());

	return std::string(out_w.begin(), out_w.end());
}

rects::wh<float> quick_print_wrapper(resources::renderable::draw_input v,
	const fstr& str,
	vec2 pos,
	unsigned wrapping_width,
	const rects::ltrb<float>* clipper)
{
	return quick_print(v.output->triangles, str, pos, wrapping_width, clipper);
}

vec2i get_text_bbox_wrapper(const fstr& str, unsigned wrapping_width) {
	return get_text_bbox(str, wrapping_width);
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

			bind_vector_wrapper<wchar_t>("wchar_t_vec"),
			bind_vector_wrapper_as_string<formatted_char>("formatted_text"),
			
			luabind::def("wchar_vec_to_str", wchar_vec_to_str),
			luabind::def("towchar", towchar),
			luabind::def("towchar_vec", towchar_vec),
			luabind::def("get_text_bbox", get_text_bbox_wrapper),
			luabind::def("quick_print_text", quick_print_wrapper)
			
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