#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/scriptable_info.h"

namespace bindings {
	luabind::scope _script() {
		return
			luabind::class_<resources::lua_state_wrapper>("lua_state_wrapper"),

			luabind::class_<script>("script")
			.def(luabind::constructor<resources::lua_state_wrapper&>())
			.def_readwrite("reload_scene_when_modified", &script::reload_scene_when_modified)
			.def("add_reload_dependant", &script::add_reload_dependant)
			.def("associate_filename", (void (script::*)(const std::string&))&script::associate_filename)
			.def("call", &script::call);
	}
}