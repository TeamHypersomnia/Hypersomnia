#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "graphics/pixel.h"

namespace bindings {
	luabind::scope _rgba() {
		return
			luabind::class_<graphics::pixel_32>("rgba")
			.def(luabind::constructor<graphics::color, graphics::color, graphics::color, graphics::color>())
			.def_readwrite("r", &graphics::pixel_32::r)
			.def_readwrite("g", &graphics::pixel_32::g)
			.def_readwrite("b", &graphics::pixel_32::b)
			.def_readwrite("a", &graphics::pixel_32::a);
	}
}