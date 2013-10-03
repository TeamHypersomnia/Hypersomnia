#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../resources/scriptable_info.h"

namespace bindings {
	luabind::scope _script() {
		return
			luabind::class_<script>("script")
			.def(luabind::constructor<>())
			.def_readwrite("reload_scene_when_modified", &script::reload_scene_when_modified)
			.def("add_reload_dependant", &script::add_reload_dependant)
			.def("associate_filename", (void (script::*)(const std::string&))&script::associate_filename)
			.def("call", &script::call);
	}
}