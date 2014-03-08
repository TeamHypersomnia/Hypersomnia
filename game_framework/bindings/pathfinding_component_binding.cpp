#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/pathfinding_component.h"
#include "../systems/pathfinding_system.h"

namespace bindings {
	luabind::scope _pathfinding_component() {
		return
			
			luabind::class_<pathfinding::navigation_hint>("navigation_hint")
			.def(luabind::constructor<>())
			.def_readwrite("enabled", &pathfinding::navigation_hint::enabled)
			.def_readwrite("origin", &pathfinding::navigation_hint::origin)
			.def_readwrite("target", &pathfinding::navigation_hint::target)
			,

			luabind::class_<pathfinding>("pathfinding_component")
			.def(luabind::constructor<>())
			.def("start_pathfinding", &pathfinding::start_pathfinding)
			.def("start_exploring", &pathfinding::start_exploring)
			.def("is_still_exploring", &pathfinding::is_still_exploring)
			.def("get_current_navigation_target", &pathfinding::get_current_navigation_target)
			.def("get_current_target", &pathfinding::get_current_target)
			.def("clear_pathfinding_info", &pathfinding::clear_pathfinding_info)
			.def("is_still_pathfinding", &pathfinding::is_still_pathfinding)
			.def("exists_through_undiscovered_visible", &pathfinding::exists_through_undiscovered_visible)
			.def("reset_persistent_navpoint", &pathfinding::reset_persistent_navpoint)
			.def("clear_internal_data", &pathfinding::clear_internal_data)
			.def_readwrite("custom_exploration_hint", &pathfinding::custom_exploration_hint)
			.def_readwrite("favor_velocity_parallellness", &pathfinding::favor_velocity_parallellness)
			.def_readwrite("enable_backtracking", &pathfinding::enable_backtracking)
			.def_readwrite("force_touch_sensors", &pathfinding::force_touch_sensors)
			.def_readwrite("rotate_navpoints", &pathfinding::rotate_navpoints)
			.def_readwrite("target_offset", &pathfinding::target_offset)
			.def_readwrite("eye_offset", &pathfinding::eye_offset)
			.def_readwrite("first_priority_navpoint_check", &pathfinding::first_priority_navpoint_check)
			.def_readwrite("force_touch_sensors", &pathfinding::force_touch_sensors)
			.def_readwrite("force_persistent_navpoints", &pathfinding::force_persistent_navpoints)
			.def_readwrite("distance_navpoint_hit", &pathfinding::distance_navpoint_hit)
			.def_readwrite("starting_ignore_discontinuities_shorter_than", &pathfinding::starting_ignore_discontinuities_shorter_than)
			.def_readwrite("target_visibility_condition", &pathfinding::target_visibility_condition)
			.def_readwrite("enable_session_rollbacks", &pathfinding::enable_session_rollbacks)
			.def_readwrite("mark_touched_as_discovered", &pathfinding::mark_touched_as_discovered)
			; 
	}
}