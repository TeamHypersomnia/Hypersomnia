#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/physics_component.h"

namespace bindings {
	luabind::scope _physics_component() {
		return
			luabind::class_<physics>("physics_component")
			.def(luabind::constructor<>())
			.def_readwrite("body", &physics::body);
	}
}