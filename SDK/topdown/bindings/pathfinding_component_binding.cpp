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
			
			luabind::class_<pathfinding>("pathfinding_component")
			.def(luabind::constructor<>())
			.def("start_pathfinding", &pathfinding::start_pathfinding)
			.def("get_current_navigation_target", &pathfinding::get_current_navigation_target)
			.def("clear_pathfinding_info", &pathfinding::clear_pathfinding_info)
			.def("is_still_pathfinding", &pathfinding::is_still_pathfinding)
			.def_readwrite("enable_backtracking", &pathfinding::enable_backtracking)
			.def_readwrite("target_offset", &pathfinding::target_offset)
			.def_readwrite("distance_navpoint_hit", &pathfinding::distance_navpoint_hit)
			; 
	}
}