#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/transform_component.h"

namespace bindings {
	luabind::scope _transform_component() {
		return
			(
			luabind::class_<transform::state<>>("transform_state")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const transform::state<>&>())
			.def_readwrite("pos", &transform::state<>::pos)
			.def_readwrite("rotation", &transform::state<>::rotation)
			,

			luabind::class_<transform>("transform_component")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const transform&>())
			.def_readwrite("current", &transform::current)
			.def_readwrite("previous", &transform::previous)
			);
	}
}