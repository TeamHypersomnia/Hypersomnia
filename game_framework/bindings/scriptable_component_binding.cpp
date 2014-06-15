#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/scriptable_component.h"

namespace bindings {
	luabind::scope _scriptable_component() {
		return(
			luabind::class_<scriptable>("scriptable_component")
			.def(luabind::constructor<>())
			.def_readwrite("subscribe_to_collisions", &scriptable::subscribe_to_collisions)
			.def_readwrite("script_data", &scriptable::script_data)
			//.enum_("script_type")[
			//	luabind::value("COLLISION_MESSAGE", scriptable::script_type::COLLISION_MESSAGE),
			//	luabind::value("DAMAGE_MESSAGE", scriptable::script_type::DAMAGE_MESSAGE),
			//	luabind::value("INTENT_MESSAGE", scriptable::script_type::INTENT_MESSAGE),
			//	luabind::value("SHOT_MESSAGE", scriptable::script_type::SHOT_MESSAGE),
			//	luabind::value("LOOP", scriptable::script_type::LOOP)
			//]
		);
	}
}