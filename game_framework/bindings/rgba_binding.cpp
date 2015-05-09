#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "graphics/pixel.h"

namespace bindings {
	luabind::scope _rgba() {
		return
			luabind::class_<graphics::hsv>("hsv")
			.def(luabind::constructor<double, double, double>())
			.def_readwrite("h", &graphics::hsv::h)
			.def_readwrite("s", &graphics::hsv::s)
			.def_readwrite("v", &graphics::hsv::v),

			luabind::class_<graphics::pixel_32>("rgba")
			.def(luabind::constructor<graphics::color, graphics::color, graphics::color, graphics::color>())
			.def("get_hsv", &graphics::pixel_32::get_hsv)
			.def("set_hsv", &graphics::pixel_32::set_hsv)
			.def_readwrite("r", &graphics::pixel_32::r)
			.def_readwrite("g", &graphics::pixel_32::g)
			.def_readwrite("b", &graphics::pixel_32::b)
			.def_readwrite("a", &graphics::pixel_32::a);
	}
}