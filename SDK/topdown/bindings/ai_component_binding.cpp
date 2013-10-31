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
			.def_readwrite("draw_discontinuities", &ai_system::draw_discontinuities),

			luabind::class_<ai>("ai_component")
			.def(luabind::constructor<>())
			.def_readwrite("visibility_square_side", &ai::visibility_square_side)
			.def_readwrite("visibility_color", &ai::visibility_color)
			.def_readwrite("postprocessing_subject", &ai::postprocessing_subject)
			;
	}
}