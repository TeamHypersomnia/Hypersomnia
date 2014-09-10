#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/vec2d.h"
#include <luabind/operator.hpp>

template<class T>
luabind::scope get_scope(const char* name) {
	return luabind::class_<vec2<T>>(name)
		.def(luabind::constructor<T, T>())
		.def(luabind::constructor<>())
		.def(luabind::constructor<const vec2<T>&>())
		.def(luabind::const_self * T())
		.def(luabind::const_self * luabind::const_self)
		.def(luabind::const_self / T())
		.def(luabind::const_self / luabind::const_self)
		.def(luabind::const_self + T())
		.def(luabind::const_self + luabind::const_self)
		.def(luabind::const_self - T())
		.def(luabind::const_self - luabind::const_self)
		.def("normalize", &vec2<T>::normalize)
		.def("length", &vec2<T>::length)
		.def("length_sq", &vec2<T>::length_sq)
		.def("get_degrees", &vec2<T>::get_degrees)
		.def("clamp", &vec2<T>::clamp<T>)
		.def("get_radians", &vec2<T>::get_radians)
		.def("set_length", &vec2<T>::set_length)
		.def("cross", &vec2<T>::cross)
		.def("set_from_degrees", &vec2<T>::set_from_degrees)
		.def("non_zero", &vec2<T>::non_zero)
		.def("perpendicular_cw", &vec2<T>::perpendicular_cw)
		.def("rotate", &vec2<>::rotate<vec2<T>>)
		.def_readwrite("x", &vec2<T>::x)
		.def_readwrite("y", &vec2<T>::y)

		.scope
		[
			luabind::def("from_degrees", &vec2<T>::from_degrees<T>),
			luabind::def("random_on_circle", &vec2<T>::random_on_circle<T>),
			luabind::def("rotated", &augs::from_rotation<vec2<T>, T>)
		];
}

namespace bindings {
	luabind::scope _vec2() {
		return get_scope<float>("vec2"), get_scope<int>("vec2_i");
	}
}