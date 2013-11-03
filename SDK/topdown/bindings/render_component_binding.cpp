#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../systems/render_system.h"
#include "../components/render_component.h"
#include "../resources/render_info.h"

namespace bindings {
	luabind::scope _render_component() {
		return
			luabind::class_<render_system>("_render_system")
			.def_readwrite("visibility_expansion", &render_system::visibility_expansion)
			.def_readwrite("max_visibility_expansion_distance", &render_system::max_visibility_expansion_distance)
			.def_readwrite("draw_steering_forces", &render_system::draw_steering_forces)
			.def_readwrite("draw_substeering_forces", &render_system::draw_substeering_forces)
			.def_readwrite("draw_velocities", &render_system::draw_velocities)
			.def_readwrite("draw_visibility", &render_system::draw_visibility),

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