#pragma once
#include "stdafx.h"
#include "game/bindings/bindings.h"

#include "misc/smooth_value_field.h"

namespace bindings {
	luabind::scope _utilities() {
		return
			luabind::class_<smooth_value_field>("smooth_value_field")
			.def(luabind::constructor<>())
			.def("tick", &smooth_value_field::tick)
			.def_readwrite("discrete_value", &smooth_value_field::discrete_value)
			.def_readwrite("value", &smooth_value_field::value)
			.def_readwrite("target_value", &smooth_value_field::target_value)
			.def_readwrite("smoothing_average_factor", &smooth_value_field::smoothing_average_factor)
			.def_readwrite("averages_per_sec", &smooth_value_field::averages_per_sec)
			;
	}
}