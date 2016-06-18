#pragma once
#include "stdafx.h"
#include "game/bindings/bindings.h"

#include "augs/scripting/script.h"

namespace bindings {
	luabind::scope _script() {
		return
			luabind::class_<lua_state_wrapper>("lua_state_wrapper"),

			luabind::class_<script>("script")
			.def(luabind::constructor<lua_state_wrapper&>())
			.def("associate_filename", (void (script::*)(const std::string&))&script::associate_filename)
			.def("call", &script::call);
	}
}