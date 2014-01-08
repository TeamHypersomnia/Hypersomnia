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
			.def(luabind::constructor<const vec2<>&>())
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
			.def("set_length", &vec2<>::set_length) 
			.def("set_from_degrees", &vec2<>::set_from_degrees)
			.def("non_zero", &vec2<>::non_zero)
			.def("rotate", &vec2<>::rotate<vec2<>>)
			.def_readwrite("x", &vec2<>::x)
			.def_readwrite("y", &vec2<>::y)

			.scope
			[
				luabind::def("from_degrees", &vec2<>::from_degrees<float>),
				luabind::def("random_on_circle", &vec2<>::random_on_circle<float>),
				luabind::def("rotated", &augmentations::from_rotation<vec2<>, float>)
			]
			;
	}
}