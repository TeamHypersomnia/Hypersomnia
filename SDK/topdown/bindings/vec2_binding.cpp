#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/vec2d.h"
#include <luabind/operator.hpp>

namespace bindings {
	luabind::scope _vec2() {
		return 
			
			luabind::class_<vec2<>>("vec2")
			.def(luabind::constructor<float, float>())
			.def(luabind::constructor<>())
			.def(luabind::const_self * float())
			.def(luabind::const_self * luabind::const_self)
			.def(luabind::const_self / float())
			.def(luabind::const_self / luabind::const_self)
			.def(luabind::const_self + float())
			.def(luabind::const_self + luabind::const_self)
			.def(luabind::const_self - float())
			.def(luabind::const_self - luabind::const_self)
			.def("normalize", &vec2<>::normalize)
			.def("length", &vec2<>::length)
			.def("length_sq", &vec2<>::length_sq)
			.def("get_degrees", &vec2<>::get_degrees)
			.def("get_radians", &vec2<>::get_radians)
			.def("set_from_degrees", &vec2<>::set_from_degrees)
			.def_readwrite("x", &vec2<>::x)
			.def_readwrite("y", &vec2<>::y);
	}
}