#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/lookat_component.h"

namespace bindings {
	luabind::scope _lookat_component() {
		return
			luabind::class_<lookat>("lookat_component")
			.def(luabind::constructor<>())
			.def_readwrite("look_mode", &lookat::look_mode)
			.def_readwrite("target", &lookat::target)
			.enum_("chase_type")[
				luabind::value("POSITION", lookat::look_type::POSITION),
				luabind::value("VELOCITY", lookat::look_type::VELOCITY)
			];
	}
}