#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/ai_component.h"
#include "../systems/ai_system.h"

namespace bindings {
	luabind::scope _ai_component() {
		return
			luabind::class_<ai_system>("_ai_system")
			.def_readwrite("draw_cast_rays", &ai_system::draw_cast_rays)
			.def_readwrite("draw_triangle_edges", &ai_system::draw_triangle_edges)
			.def_readwrite("draw_discontinuities", &ai_system::draw_discontinuities)
			.def_readwrite("draw_memorised_walls", &ai_system::draw_memorised_walls),

			luabind::class_<ai::visibility>("visibility")
			.def(luabind::constructor<>())
			.def_readwrite("filter", &ai::visibility::filter)
			.def_readwrite("square_side", &ai::visibility::square_side)
			.def_readwrite("color", &ai::visibility::color)
			.def_readwrite("postprocessing_subject", &ai::visibility::postprocessing_subject)
			.enum_("constants")
			[
				luabind::value("OBSTACLE_AVOIDANCE", ai::visibility::OBSTACLE_AVOIDANCE),
				luabind::value("DYNAMIC_PATHFINDING", ai::visibility::DYNAMIC_PATHFINDING)
			],
			
			luabind::class_<ai>("ai_component")
			.def(luabind::constructor<>())
			.def("add_request", &ai::add_request)
			.def("get_visibility", &ai::get_visibility)
			;
	}
}