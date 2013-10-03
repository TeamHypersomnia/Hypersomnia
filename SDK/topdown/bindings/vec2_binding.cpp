#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "math/vec2d.h"

namespace bindings {
	luabind::scope _vec2() {
		return 
			
			luabind::class_<vec2<>>("vec2")
			.def(luabind::constructor<float, float>())
			.def(luabind::constructor<>())
			.def_readwrite("x", &vec2<>::x)
			.def_readwrite("y", &vec2<>::y);
	}
}