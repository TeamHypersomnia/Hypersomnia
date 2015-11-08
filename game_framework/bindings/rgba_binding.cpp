#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "graphics/pixel.h"

namespace bindings {
	luabind::scope _rgba() {
		return
			luabind::class_<hsv>("hsv")
			.def(luabind::constructor<double, double, double>())
			.def_readwrite("h", &hsv::h)
			.def_readwrite("s", &hsv::s)
			.def_readwrite("v", &hsv::v),

			luabind::class_<pixel_32>("rgba")
			.def(luabind::constructor<color, color, color, color>())
			.def("get_hsv", &pixel_32::get_hsv)
			.def("set_hsv", &pixel_32::set_hsv)
			.def_readwrite("r", &pixel_32::r)
			.def_readwrite("g", &pixel_32::g)
			.def_readwrite("b", &pixel_32::b)
			.def_readwrite("a", &pixel_32::a);
	}
}