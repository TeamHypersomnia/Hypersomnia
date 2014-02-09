#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/pathfinding_component.h"
#include "../systems/pathfinding_system.h"

namespace bindings {
	luabind::scope _pathfinding_component() {
		return
			luabind::class_<pathfinding_system>("_pathfinding_system")
			.def_readwrite("epsilon_max_segment_difference", &pathfinding_system::epsilon_max_segment_difference)
			.def_readwrite("epsilon_distance_visible_point", &pathfinding_system::epsilon_distance_visible_point)
			.def_readwrite("draw_memorised_walls", &pathfinding_system::draw_memorised_walls)
			.def_readwrite("draw_undiscovered", &pathfinding_system::draw_undiscovered)
			.def_readwrite("epsilon_distance_the_same_vertex", &pathfinding_system::epsilon_distance_the_same_vertex)
			, 
			
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
			.def_readwrite("custom_exploration_hint", &pathfinding::custom_exploration_hint)
			.def_readwrite("favor_velocity_parallellness", &pathfinding::favor_velocity_parallellness)
			.def_readwrite("enable_backtracking", &pathfinding::enable_backtracking)
			.def_readwrite("force_touch_sensors", &pathfinding::force_touch_sensors)
			.def_readwrite("rotate_navpoints", &pathfinding::rotate_navpoints)
			.def_readwrite("target_offset", &pathfinding::target_offset)
			.def_readwrite("first_priority_navpoint_check", &pathfinding::first_priority_navpoint_check)
			.def_readwrite("force_touch_sensors", &pathfinding::force_touch_sensors)
			.def_readwrite("force_persistent_navpoints", &pathfinding::force_persistent_navpoints)
			.def_readwrite("distance_navpoint_hit", &pathfinding::distance_navpoint_hit)
			.def_readwrite("starting_ignore_discontinuities_shorter_than", &pathfinding::starting_ignore_discontinuities_shorter_than)
			; 
	}
}