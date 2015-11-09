#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/transform_component.h"

namespace bindings {
	luabind::scope _transform_component() {
		return
			(
			luabind::class_<transform>("transform_component")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const transform&>())
			.def_readwrite("pos", &transform::pos)
			.def_readwrite("rotation", &transform::rotation)
			);
	}
}