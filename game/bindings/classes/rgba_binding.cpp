#pragma once
#include "stdafx.h"
#include "game/bindings/bindings.h"

#include "graphics/pixel.h"

namespace bindings {
	luabind::scope _rgba() {
		return
			luabind::class_<hsv>("hsv")
			.def(luabind::constructor<double, double, double>())
			.def_readwrite("h", &hsv::h)
			.def_readwrite("s", &hsv::s)
			.def_readwrite("v", &hsv::v),

			luabind::class_<rgba>("rgba")
			.def(luabind::constructor<rgba_channel, rgba_channel, rgba_channel, rgba_channel>())
			.def("get_hsv", &rgba::get_hsv)
			.def("set_hsv", &rgba::set_hsv)
			.def_readwrite("r", &rgba::r)
			.def_readwrite("g", &rgba::g)
			.def_readwrite("b", &rgba::b)
			.def_readwrite("a", &rgba::a);
	}
}