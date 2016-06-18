#pragma once
#include "stdafx.h"
#include "game/bindings/bindings.h"

#include "augs/misc/value_animator.h"

namespace bindings {
	luabind::scope _value_animator() {
		return
			luabind::class_<value_animator>("value_animator")
			.def(luabind::constructor<float, float, float>())
			.def("set_linear", &value_animator::set_linear)
			.def("set_quadratic", &value_animator::set_quadratic)
			.def("set_sinusoidal", &value_animator::set_sinusoidal)
			.def("set_hyperbolic", &value_animator::set_hyperbolic)
			.def("set_logarithmic", &value_animator::set_logarithmic)
			.def("set_exponential", &value_animator::set_exponential)
			.def("get_animated", &value_animator::get_animated)
			.def("has_finished", &value_animator::has_finished)
			.def("start", &value_animator::start);
	}
}