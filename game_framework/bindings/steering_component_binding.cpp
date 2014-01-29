#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/steering_component.h"

namespace bindings {
	luabind::scope _steering_component() {
		return(
			luabind::class_<steering::target_info>("target_info")
			.def(luabind::constructor<>())
			.def("set", (void (steering::target_info::*)(const entity_ptr&))&steering::target_info::set)
			.def("set", (void (steering::target_info::*)(vec2<>, vec2<>))&steering::target_info::set)
			.def_readwrite("is_set", &steering::target_info::is_set)
			,
			
			luabind::class_<steering::behaviour_state>("behaviour_state")
			.def(luabind::constructor<steering::behaviour*>())
			.def_readwrite("subject_behaviour", &steering::behaviour_state::subject_behaviour)
			.def_readwrite("target", &steering::behaviour_state::target)
			.def_readwrite("target_from", &steering::behaviour_state::target_from)
			.def_readwrite("last_output_force", &steering::behaviour_state::last_output_force)
			.def_readwrite("last_estimated_target_position", &steering::behaviour_state::last_estimated_target_position)
			.def_readwrite("enabled", &steering::behaviour_state::enabled)
			.def_readwrite("weight_multiplier", &steering::behaviour_state::weight_multiplier)
			.def_readwrite("current_wander_angle", &steering::behaviour_state::current_wander_angle),

			luabind::class_<steering::behaviour>("steering_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("force_color", &steering::behaviour::force_color)
			.def_readwrite("max_force_applied", &steering::behaviour::max_force_applied)
			.def_readwrite("weight", &steering::behaviour::weight),


			luabind::class_<steering::directed, steering::behaviour>("directed_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("radius_of_effect", &steering::directed::radius_of_effect)
			.def_readwrite("max_target_future_prediction_ms", &steering::directed::max_target_future_prediction_ms),

			luabind::class_<steering::avoidance, steering::behaviour>("avoidance_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("intervention_time_ms", &steering::avoidance::intervention_time_ms)
			.def_readwrite("max_intervention_length", &steering::avoidance::max_intervention_length)
			.def_readwrite("avoidance_rectangle_width", &steering::avoidance::avoidance_rectangle_width)
			,

			luabind::class_<steering::flocking, steering::behaviour>("flocking_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("group", &steering::flocking::group)
			.def_readwrite("square_side", &steering::flocking::square_side)
			.def_readwrite("field_of_vision_degrees", &steering::flocking::field_of_vision_degrees)
			,

			luabind::class_<steering::separation, steering::flocking>("separation_behaviour")
			.def(luabind::constructor<>())
			,

			luabind::class_<steering::seek, steering::directed>("seek_behaviour")
			.def(luabind::constructor<>()),

			luabind::class_<steering::flee, steering::directed>("flee_behaviour")
			.def(luabind::constructor<>()),

			luabind::class_<steering::wander, steering::behaviour>("wander_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("circle_radius", &steering::wander::circle_radius)
			.def_readwrite("circle_distance", &steering::wander::circle_distance)
			.def_readwrite("displacement_degrees", &steering::wander::displacement_degrees)
			,

			luabind::class_<steering::containment, steering::avoidance>("containment_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("randomize_rays", &steering::containment::randomize_rays)
			.def_readwrite("only_threats_in_OBB", &steering::containment::only_threats_in_OBB)
			.def_readwrite("ray_count", &steering::containment::ray_count)
			.def_readwrite("ray_filter", &steering::containment::ray_filter)
			,

			luabind::class_<steering::obstacle_avoidance, steering::avoidance>("obstacle_avoidance_behaviour")
			.def(luabind::constructor<>())
			.def_readwrite("navigation_correction", &steering::obstacle_avoidance::navigation_correction)
			.def_readwrite("navigation_seek", &steering::obstacle_avoidance::navigation_seek)
			.def_readwrite("ignore_discontinuities_narrower_than", &steering::obstacle_avoidance::ignore_discontinuities_narrower_than)
			.def_readwrite("visibility_type", &steering::obstacle_avoidance::visibility_type)
			,

			luabind::class_<steering>("steering_component")
			.def(luabind::constructor<>())
			.def("add_behaviour", &steering::add_behaviour)
			.def("clear_behaviours", &steering::clear_behaviours)
			.def_readwrite("max_resultant_force", &steering::max_resultant_force)
			);
	}
}
