#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../systems/render_system.h"
#include "../components/render_component.h"
#include "../resources/render_info.h"

namespace bindings {
	luabind::scope _render_component() {
		return
			luabind::class_<render_system::debug_line>("debug_line")
			.def(luabind::constructor<vec2, vec2, graphics::pixel_32>())
			,
			luabind::class_<resources::buffer>("triangle_buffer"),

		

			luabind::class_<render>("render_component")
			.def(luabind::constructor<>())
			.def("get_sprite", &render::get_renderable<resources::sprite>)
			.def("get_polygon", &render::get_renderable<resources::polygon>)
			.def_readwrite("mask", &render::mask)
			.def_readwrite("layer", &render::layer)
			.def_readwrite("model", &render::model)
			.def_readwrite("last_screen_pos", &render::last_screen_pos)
			.def_readwrite("was_drawn", &render::was_drawn)
			.def_readwrite("flip_horizontally", &render::flip_horizontally)
			.def_readwrite("flip_vertically", &render::flip_vertically)
			.def_readwrite("absolute_transform", &render::absolute_transform)
			.enum_("mask_type")[
				luabind::value("WORLD", render::mask_type::WORLD),
				luabind::value("GUI", render::mask_type::GUI)
			];
	}
}