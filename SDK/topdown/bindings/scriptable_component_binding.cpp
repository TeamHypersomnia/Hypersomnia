#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/scriptable_component.h"

namespace bindings {
	luabind::scope _scriptable_component() {
		return(
			luabind::class_<scriptable_info>("scriptable_info")
			.def(luabind::constructor<>())
			.def("set", &scriptable_info::add)
			.def("at", &scriptable_info::get),

			luabind::class_<scriptable>("scriptable_component")
			.def(luabind::constructor<>())
			.def_readwrite("available_scripts", &scriptable::available_scripts)
			.enum_("script_type")[
				luabind::value("COLLISION_MESSAGE", scriptable::script_type::COLLISION_MESSAGE),
				luabind::value("DAMAGE_MESSAGE", scriptable::script_type::DAMAGE_MESSAGE),
				luabind::value("INTENT_MESSAGE", scriptable::script_type::INTENT_MESSAGE),
				luabind::value("LOOP", scriptable::script_type::LOOP)
			]
		);
	}
}