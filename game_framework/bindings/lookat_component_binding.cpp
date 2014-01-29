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
			.def_readwrite("easing_mode", &lookat::easing_mode)
			.def_readwrite("smoothing_average_factor", &lookat::smoothing_average_factor)
			.def_readwrite("averages_per_sec", &lookat::averages_per_sec)
			.enum_("chase_type")[
				luabind::value("POSITION", lookat::look_type::POSITION),
				luabind::value("VELOCITY", lookat::look_type::VELOCITY),
				luabind::value("ACCELEARATION", lookat::look_type::ACCELEARATION),
				luabind::value("NONE", lookat::lookat_easing::NONE),
				luabind::value("LINEAR", lookat::lookat_easing::LINEAR),
				luabind::value("EXPONENTIAL", lookat::lookat_easing::EXPONENTIAL)
			];
	}
}