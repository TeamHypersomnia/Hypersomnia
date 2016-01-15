#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "math/vec2.h"
#include <luabind/operator.hpp>

template<class T>
luabind::scope get_scope(const char* name) {
	return luabind::class_<vec2t<T>>(name)
		.def(luabind::constructor<T, T>())
		.def(luabind::constructor<>())
		.def(luabind::constructor<const vec2t<T>&>())
		.def(luabind::const_self * T())
		.def(luabind::const_self * luabind::const_self)
		.def(luabind::const_self / T())
		.def(luabind::const_self / luabind::const_self)
		.def(luabind::const_self + T())
		.def(luabind::const_self + luabind::const_self)
		.def(luabind::const_self - T())
		.def(luabind::const_self - luabind::const_self)
		.def("normalize", &vec2t<T>::normalize)
		.def("length", &vec2t<T>::length)
		.def("length_sq", &vec2t<T>::length_sq)
		.def("degrees", &vec2t<T>::degrees)
		.def("clamp", &vec2t<T>::clamp<T>)
		.def("radians", &vec2t<T>::radians)
		.def("set_length", &vec2t<T>::set_length)
		.def("cross", &vec2t<T>::cross)
		.def("set_from_degrees", &vec2t<T>::set_from_degrees)
		.def("non_zero", &vec2t<T>::non_zero)
		.def("perpendicular_cw", &vec2t<T>::perpendicular_cw)
		.def("rotate", &vec2t<T>::rotate<vec2t<T>>)
		.def_readwrite("x", &vec2t<T>::x)
		.def_readwrite("y", &vec2t<T>::y)

		.scope
		[
			luabind::def("random_on_circle", &vec2t<T>::random_on_circle<T>),
			luabind::def("rotated", &augs::from_rotation<vec2t<T>, T>)
		];
}

namespace bindings {
	luabind::scope _vec2() {
		return get_scope<float>("vec2"), get_scope<int>("vec2_i"), get_scope<double>("vec2_d");
	}
}