#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../resources/particle_emitter_info.h"

namespace bindings {
	luabind::scope _particle() {
		return
			luabind::class_<particle>("particle")
			.def(luabind::constructor<>())
			.def_readwrite("pos", &particle::pos)
			.def_readwrite("vel", &particle::vel)
			.def_readwrite("acc", &particle::acc)
			.def_readwrite("model", &particle::face)
			.def_readwrite("rotation", &particle::rotation)
			.def_readwrite("rotation_speed", &particle::rotation_speed)
			.def_readwrite("linear_damping", &particle::linear_damping)
			.def_readwrite("angular_damping", &particle::angular_damping)
			.def_readwrite("lifetime_ms", &particle::lifetime_ms)
			.def_readwrite("max_lifetime_ms", &particle::max_lifetime_ms)
			.def_readwrite("alpha_levels", &particle::alpha_levels)
			.def_readwrite("should_disappear", &particle::should_disappear)
			.def_readwrite("ignore_rotation", &particle::ignore_rotation)
			
			;
	}
}