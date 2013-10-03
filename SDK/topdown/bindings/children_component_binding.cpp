#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/children_component.h"

namespace bindings {
	luabind::scope _children_component() {
		return
			luabind::class_<children>("children_component")
			.def(luabind::constructor<>())
			.def("add", &children::add);
	}
}