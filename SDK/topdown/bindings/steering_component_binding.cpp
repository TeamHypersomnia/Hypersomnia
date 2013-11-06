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
			.def_readwrite("enabled", &steering::behaviour::enabled)
			.def_readwrite("force_color", &steering::behaviour::force_color)
			.def_readwrite("last_estimated_pursuit_position", &steering::behaviour::last_estimated_pursuit_position)
			.def_readwrite("arrival_slowdown_radius", &steering::behaviour::arrival_slowdown_radius)
			.def_readwrite("max_target_future_prediction_ms", &steering::behaviour::max_target_future_prediction_ms)
			.def_readwrite("effective_fleeing_radius", &steering::behaviour::effective_fleeing_radius)
			.def_readwrite("intervention_time_ms", &steering::behaviour::intervention_time_ms)
			.def_readwrite("avoidance_rectangle_width", &steering::behaviour::avoidance_rectangle_width)
			.enum_("script_type")[
				luabind::value("SEEK", steering::behaviour::SEEK),
				luabind::value("FLEE", steering::behaviour::FLEE),
				luabind::value("PURSUIT", steering::behaviour::PURSUIT),
				luabind::value("EVASION", steering::behaviour::EVASION),
				luabind::value("OBSTACLE_AVOIDANCE", steering::behaviour::OBSTACLE_AVOIDANCE)
			],

			luabind::class_<steering>("steering_component")
			.def(luabind::constructor<>())
			.def("add_behaviour", &steering::add_behaviour)
			.def("clear_behaviours", &steering::clear_behaviours)
			.def_readwrite("max_resultant_force", &steering::max_resultant_force)
			);
	}
}
