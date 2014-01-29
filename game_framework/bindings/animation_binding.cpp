#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/animate_info.h"
#include "../resources/render_info.h"

namespace bindings {
	luabind::scope _animation() {
		return
			luabind::class_<animation>("animation")
			.def(luabind::constructor<>())
			.def("add_frame", &animation::add_frame<sprite>)
			.def_readwrite("loop_mode", &animation::loop_mode)
			.enum_("loop_type")[
				luabind::value("REPEAT", animation::loop_type::REPEAT),
				luabind::value("INVERSE", animation::loop_type::INVERSE),
				luabind::value("NONE", animation::loop_type::NONE)
			];
	}
}