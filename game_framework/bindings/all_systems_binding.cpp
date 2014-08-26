#pragma once
#include "stdafx.h"
#include "entity_system/world.h"
#include "bindings.h"

#include "../systems/physics_system.h"
#include "../systems/steering_system.h"
#include "../systems/movement_system.h"
#include "../systems/visibility_system.h"
#include "../systems/pathfinding_system.h"
#include "../systems/animation_system.h"
#include "../systems/camera_system.h"
#include "../systems/render_system.h"
#include "../systems/input_system.h"
#include "../systems/gun_system.h"
#include "../systems/crosshair_system.h"
#include "../systems/lookat_system.h"
#include "../systems/chase_system.h"
#include "../systems/damage_system.h"
#include "../systems/destroy_system.h"
#include "../systems/particle_group_system.h"
#include "../systems/particle_emitter_system.h"
#include "../systems/behaviour_tree_system.h"

#include "misc/vector_wrapper.h"

namespace bindings {
	luabind::scope _all_systems() {
		return
			luabind::class_<physics_system>("_physics_system")
			.def("process_entities", &physics_system::process_entities)
			.def("process_steps", &physics_system::process_steps)
			.def("enable_listener", &physics_system::enable_listener)
			.def_readwrite("timestep_multiplier", &physics_system::timestep_multiplier)
			.def_readwrite("enable_interpolation", &physics_system::enable_interpolation)
			.def_readwrite("b2world", &physics_system::b2world)
			.def_readwrite("prestepping_routine", &physics_system::prestepping_routine)
			.def_readwrite("poststepping_routine", &physics_system::poststepping_routine)
			.def("ray_cast", &physics_system::ray_cast_px)
			.def("query_aabb", (physics_system::query_output(__thiscall physics_system::*) (vec2<>, vec2<>, b2Filter*, entity*))(&physics_system::query_aabb_px))
			.def("query_body", (physics_system::query_output(__thiscall physics_system::*) (entity& subject, b2Filter*, entity*)) (&physics_system::query_body))
			.def("query_shape", (physics_system::query_output(__thiscall physics_system::*) (b2Shape*, b2Filter*, entity*)) (&physics_system::query_shape))
			.def("query_polygon", (physics_system::query_output(__thiscall physics_system::*) (const std::vector<vec2<>>&, b2Filter*, entity*)) (&physics_system::query_polygon))
			.def("push_away_from_walls", &physics_system::push_away_from_walls)
			.def("get_timestep_ms", &physics_system::get_timestep_ms)
			.def("configure_stepping", &physics_system::configure_stepping),

			luabind::class_<steering_system>("_steering_system")
			.def("process_entities", &steering_system::process_entities)
			.def("substep", &steering_system::substep)
			,
			luabind::class_<movement_system>("_movement_system")
			.def("substep", &movement_system::substep)
			.def("consume_events", &movement_system::consume_events)
			.def("process_entities", &movement_system::process_entities),
			
			luabind::class_<visibility_system>("_visibility_system")
			.def("process_entities", &visibility_system::process_entities)
			.def_readwrite("draw_cast_rays", &visibility_system::draw_cast_rays)
			.def_readwrite("draw_triangle_edges", &visibility_system::draw_triangle_edges)
			.def_readwrite("draw_discontinuities", &visibility_system::draw_discontinuities)
			.def_readwrite("draw_visible_walls", &visibility_system::draw_visible_walls)
			.def_readwrite("epsilon_ray_distance_variation", &visibility_system::epsilon_ray_distance_variation)
			.def_readwrite("epsilon_distance_vertex_hit", &visibility_system::epsilon_distance_vertex_hit)
			.def_readwrite("epsilon_threshold_obstacle_hit", &visibility_system::epsilon_threshold_obstacle_hit)
			,

			luabind::class_<pathfinding_system>("_pathfinding_system")
			.def("process_entities", &pathfinding_system::process_entities)
			.def_readwrite("epsilon_max_segment_difference", &pathfinding_system::epsilon_max_segment_difference)
			.def_readwrite("epsilon_distance_visible_point", &pathfinding_system::epsilon_distance_visible_point)
			.def_readwrite("draw_memorised_walls", &pathfinding_system::draw_memorised_walls)
			.def_readwrite("draw_undiscovered", &pathfinding_system::draw_undiscovered)
			.def_readwrite("epsilon_distance_the_same_vertex", &pathfinding_system::epsilon_distance_the_same_vertex)
			,

			luabind::class_<animation_system>("_animation_system")
			.def("consume_events", &animation_system::consume_events)
			.def("process_entities", &animation_system::process_entities),
			luabind::class_<camera_system>("_camera_system")
			.def("process_rendering", &camera_system::process_rendering)
			.def("process_entities", &camera_system::process_entities)
			.def("consume_events", &camera_system::consume_events)
			
			,

			luabind::class_<render_system>("_render_system")
			.def("process_entities", &render_system::process_entities)
			.def_readwrite("visibility_expansion", &render_system::visibility_expansion)
			.def_readwrite("max_visibility_expansion_distance", &render_system::max_visibility_expansion_distance)
			.def_readwrite("draw_steering_forces", &render_system::draw_steering_forces)
			.def_readwrite("draw_substeering_forces", &render_system::draw_substeering_forces)
			.def_readwrite("draw_velocities", &render_system::draw_velocities)
			.def_readwrite("draw_avoidance_info", &render_system::draw_avoidance_info)
			.def_readwrite("draw_wandering_info", &render_system::draw_wandering_info)
			.def_readwrite("draw_visibility", &render_system::draw_visibility)
			.def_readwrite("draw_weapon_info", &render_system::draw_weapon_info)
			.def_readwrite("debug_drawing", &render_system::debug_drawing)
			.def_readwrite("triangles", &render_system::triangles)
			.def("push_line", &render_system::push_line)
			.def("push_non_cleared_line", &render_system::push_non_cleared_line)
			.def("push_line_channel", &render_system::push_line_channel)
			.def("clear_channel", &render_system::clear_channel)
			.def("clear_non_cleared_lines", &render_system::clear_non_cleared_lines)

			.def("generate_layers", &render_system::generate_layers)
			.def("draw_layer", &render_system::draw_layer)
			.def("call_triangles", &render_system::call_triangles)
			.def("push_triangle", &render_system::push_triangle)
			.def("clear_triangles", &render_system::clear_triangles)
			.def("draw_debug_info", &render_system::draw_debug_info)
			.def("generate_triangles", &render_system::generate_triangles)
			.def("default_render", &render_system::default_render)
			.def("get_triangle_count", &render_system::get_triangle_count)
			.def("get_triangle", &render_system::get_triangle)
			,

			luabind::class_<input_system>("_input_system")
			.def("process_entities", &input_system::process_entities)
			.def_readwrite("quit_flag", &input_system::quit_flag)
			.def_readwrite("event_callback", &input_system::event_callback)
			.def("add_context", &input_system::add_context)
			.def("is_down", &input_system::is_down)
			.def("clear_contexts", &input_system::clear_contexts),

			luabind::class_<gun_system>("_gun_system")
			.def("consume_events", &gun_system::consume_events)
			.def("process_entities", &gun_system::process_entities),
			luabind::class_<crosshair_system>("_crosshair_system")
			.def("consume_events", &crosshair_system::consume_events)
			.def("process_entities", &crosshair_system::process_entities),
			luabind::class_<lookat_system>("_lookat_system")
			.def("process_entities", &lookat_system::process_entities),
			luabind::class_<chase_system>("_chase_system")
			.def("process_entities", &chase_system::process_entities),
			luabind::class_<damage_system>("_damage_system")
			.def("process_events", &damage_system::process_events)
			.def("process_entities", &damage_system::process_entities),
			luabind::class_<destroy_system>("_destroy_system")
			.def("consume_events", &destroy_system::consume_events),
			luabind::class_<particle_group_system>("_particle_group_system")
			.def("process_entities", &particle_group_system::process_entities),
			luabind::class_<particle_emitter_system>("_particle_emitter_system")
			.def("consume_events", &particle_emitter_system::consume_events),

			luabind::def("create_refreshable_particle_group", &particle_emitter_system::create_refreshable_particle_group),

			augs::misc::vector_wrapper<luabind::object>::bind_vector("luabindobject_vector"),
			
			luabind::class_<behaviour_tree_system>("_behaviour_tree_system")
			.def("process_entities", &behaviour_tree_system::process_entities)
			;
			
	}
}