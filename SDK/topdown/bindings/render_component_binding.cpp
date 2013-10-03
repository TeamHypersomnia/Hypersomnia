#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/render_component.h"
#include "../resources/render_info.h"

namespace bindings {
	luabind::scope _render_component() {
		return		
		luabind::class_<render>("render_component")
			.def(luabind::constructor<>())
			.def_readwrite("mask", &render::mask)
			.def_readwrite("layer", &render::layer)
			.property("model", &render::get_renderable<sprite>, &render::set_renderable<sprite>)
			.enum_("mask_type")[
				luabind::value("WORLD", render::mask_type::WORLD),
				luabind::value("GUI", render::mask_type::GUI)
			];
	}
}