#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/animate_info.h"
#include "../components/animate_component.h"

namespace bindings {
	luabind::scope _animate_component() {
		return
			(
			bind_map_wrapper<int, animation*>("animate_info"),

			luabind::class_<animate>("animate_component")
			.def(luabind::constructor<>())
			.def_readwrite("available_animations", &animate::available_animations)
			.def("set_current_animation_set", &animate::set_current_animation_set)
			);
	}
}