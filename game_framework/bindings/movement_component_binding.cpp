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
			.def_readwrite("force_offset", &movement::force_offset)
			.def_readwrite("animation_message", &movement::animation_message)
			.def_readwrite("moving_left", &movement::moving_left)
			.def_readwrite("moving_right", &movement::moving_right)
			.def_readwrite("moving_forward", &movement::moving_forward)
			.def_readwrite("moving_backward", &movement::moving_backward)
			.def_readwrite("axis_rotation_degrees", &movement::axis_rotation_degrees)
			.def_readwrite("input_acceleration", &movement::input_acceleration)
			.def_readwrite("braking_damping", &movement::braking_damping)
			.def_readwrite("max_speed_animation", &movement::max_speed_animation)
			.def_readwrite("inverse_thrust_brake", &movement::inverse_thrust_brake)
			.def_readwrite("max_speed", &movement::max_speed)
			.def_readwrite("thrust_parallel_to_ground_length", &movement::thrust_parallel_to_ground_length)
			.def_readwrite("ground_filter", &movement::ground_filter)
			.def_readwrite("requested_movement", &movement::requested_movement)
			.def_readwrite("sidescroller_setup", &movement::sidescroller_setup)
			.def_readwrite("air_resistance", &movement::air_resistance);
	}
}