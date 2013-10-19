#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/ai_component.h"

namespace bindings {
	luabind::scope _ai_component() {
		return
			luabind::class_<ai>("ai_component")
			.def(luabind::constructor<>())
			.def_readwrite("visibility_square_side", &ai::visibility_square_side);
	}
}