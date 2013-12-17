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
			.def(luabind::constructor<vec2<>, vec2<>, graphics::pixel_32>())
			,

			luabind::class_<render_system>("_render_system")
			.def_readwrite("visibility_expansion", &render_system::visibility_expansion)
			.def_readwrite("max_visibility_expansion_distance", &render_system::max_visibility_expansion_distance)
			.def_readwrite("draw_steering_forces", &render_system::draw_steering_forces)
			.def_readwrite("draw_substeering_forces", &render_system::draw_substeering_forces)
			.def_readwrite("draw_velocities", &render_system::draw_velocities)
			.def_readwrite("draw_avoidance_info", &render_system::draw_avoidance_info)
			.def_readwrite("draw_wandering_info", &render_system::draw_wandering_info)
			.def_readwrite("draw_visibility", &render_system::draw_visibility)
			.def_readwrite("debug_drawing", &render_system::debug_drawing)
			.def("push_line", &render_system::push_line)
			.def("push_non_cleared_line", &render_system::push_non_cleared_line)
			.def("clear_non_cleared_lines", &render_system::clear_non_cleared_lines)
			, 

			luabind::class_<render>("render_component")
			.def(luabind::constructor<>())
			.def_readwrite("mask", &render::mask)
			.def_readwrite("layer", &render::layer)
			.def_readwrite("model", &render::model)
			.enum_("mask_type")[
				luabind::value("WORLD", render::mask_type::WORLD),
				luabind::value("GUI", render::mask_type::GUI)
			];
	}
}