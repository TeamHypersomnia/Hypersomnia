#pragma once
#include "stdafx.h"
#include "bindings.h"
#include <Box2D\Dynamics\b2Fixture.h>

namespace bindings {
	luabind::scope _b2Filter() {
		return
			luabind::class_<b2Filter>("b2Filter")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const b2Filter&>())
			.def_readwrite("categoryBits", &b2Filter::categoryBits)
			.def_readwrite("maskBits", &b2Filter::maskBits)
			.def_readwrite("groupIndex", &b2Filter::groupIndex);
	}
}