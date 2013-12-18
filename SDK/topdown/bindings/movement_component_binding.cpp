#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/movement_component.h"

namespace bindings {
	luabind::scope _movement_component() {
		return
			luabind::class_<movement>("movement_component")
			.def(luabind::constructor<>())
			.def("add_animation_receiver", &movement::add_animation_receiver)
			.def_readwrite("moving_left", &movement::moving_left)
			.def_readwrite("moving_right", &movement::moving_right)
			.def_readwrite("moving_forward", &movement::moving_forward)
			.def_readwrite("moving_backward", &movement::moving_backward)
			.def_readwrite("input_acceleration", &movement::input_acceleration)
			.def_readwrite("braking_damping", &movement::braking_damping)
			.def_readwrite("max_speed_animation", &movement::max_speed_animation)
			.def_readwrite("max_speed", &movement::max_speed)
			.def_readwrite("air_resistance", &movement::air_resistance);
	}
}