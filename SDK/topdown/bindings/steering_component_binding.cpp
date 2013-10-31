#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/steering_component.h"

namespace bindings {
	luabind::scope _steering_component() {
		return(
			luabind::class_<steering::behaviour>("steering_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("behaviour_type", &steering::behaviour::behaviour_type)
			.def_readwrite("current_target", &steering::behaviour::current_target)
			.def_readwrite("max_force_applied", &steering::behaviour::max_force_applied)
			.def_readwrite("weight", &steering::behaviour::weight)
			.def_readwrite("erase_when_target_reached", &steering::behaviour::erase_when_target_reached)
			.enum_("script_type")[
				luabind::value("SEEK", steering::behaviour::SEEK),
					luabind::value("FLEE", steering::behaviour::FLEE),
					luabind::value("ARRIVAL", steering::behaviour::ARRIVAL),
					luabind::value("PURSUIT", steering::behaviour::PURSUIT)
			],

			luabind::class_<steering>("steering_component")
			.def(luabind::constructor<>())
			.def("add_behaviour", &steering::add_behaviour)
			.def("clear_behaviours", &steering::clear_behaviours)
			);
	}
}
