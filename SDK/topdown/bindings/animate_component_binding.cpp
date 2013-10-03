#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../resources/animate_info.h"
#include "../components/animate_component.h"

namespace bindings {
	luabind::scope _animate_component() {
		return
			(
			luabind::class_<animate_info>("animate_info")
			.def(luabind::constructor<>())
			.def("add", &animate_info::add),

			luabind::class_<animate>("animate_component")
			.def(luabind::constructor<>())
			.def_readwrite("available_animations", &animate::available_animations)
			);
	}
}