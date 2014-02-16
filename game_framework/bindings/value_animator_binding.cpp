#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "utilities/misc/value_animator.h"

namespace bindings {
	luabind::scope _value_animator() {
		return
			luabind::class_<misc::animator>("value_animator")
			.def(luabind::constructor<float, float, float>())
			.def("set_linear", &misc::animator::set_linear)
			.def("set_quadratic", &misc::animator::set_quadratic)
			.def("set_sinusoidal", &misc::animator::set_sinusoidal)
			.def("set_hyperbolic", &misc::animator::set_hyperbolic)
			.def("set_logarithmic", &misc::animator::set_logarithmic)
			.def("set_exponential", &misc::animator::set_exponential)
			.def("get_animated", &misc::animator::get_animated)
			.def("start", &misc::animator::start);
	}
}