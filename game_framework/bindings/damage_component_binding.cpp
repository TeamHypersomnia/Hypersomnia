#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/damage_component.h"

namespace bindings {
	luabind::scope _damage_component() {
		return
			luabind::class_<damage>("damage_component")
			.def(luabind::constructor<>())
			.def_readwrite("amount", &damage::amount)
			.def_readwrite("sender", &damage::sender)
			.def_readwrite("starting_point", &damage::starting_point)
			.def_readwrite("max_distance", &damage::max_distance);
	}
}