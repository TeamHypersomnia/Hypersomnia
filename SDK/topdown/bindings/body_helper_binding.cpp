#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../game/body_helper.h"

namespace bindings {
	luabind::scope _body_helper() {
		return
			luabind::def("create_physics_component", create_physics_component_str);
	}
}