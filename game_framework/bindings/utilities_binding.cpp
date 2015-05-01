#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "misc/smooth_value_field.h"

namespace bindings {
	luabind::scope _utilities() {
		return
			luabind::class_<misc::smooth_value_field>("smooth_value_field")
			.def(luabind::constructor<>())
			.def("tick", &misc::smooth_value_field::tick)
			.def_readwrite("discrete_value", &misc::smooth_value_field::discrete_value)
			.def_readwrite("value", &misc::smooth_value_field::value)
			.def_readwrite("target_value", &misc::smooth_value_field::target_value)
			.def_readwrite("smoothing_average_factor", &misc::smooth_value_field::smoothing_average_factor)
			.def_readwrite("averages_per_sec", &misc::smooth_value_field::averages_per_sec)
			;
	}
}