#pragma once
#include "stdafx.h"
#include "entity_system/world.h"
#include "bindings.h"
#include "bind_callbacks.h"

#include "game_framework/all_system_includes.h"

#include "misc/vector_wrapper.h"

namespace bindings {
	luabind::scope _all_systems() {
		return
			luabind::class_<physics_system>("_physics_system")
			.def("enable_listener", &physics_system::enable_listener)
			.def_readwrite("b2world", &physics_system::b2world)
			.def("ray_cast", &physics_system::ray_cast_px)
			.def("query_aabb", (physics_system::query_output(__thiscall physics_system::*) (vec2, vec2, b2Filter*, entity_id))(&physics_system::query_aabb_px))
			.def("query_body", (physics_system::query_output(__thiscall physics_system::*) (entity_id subject, b2Filter*, entity_id)) (&physics_system::query_body))
			.def("query_shape", (physics_system::query_output(__thiscall physics_system::*) (b2Shape*, b2Filter*, entity_id)) (&physics_system::query_shape))
			.def("query_polygon", (physics_system::query_output(__thiscall physics_system::*) (const std::vector<vec2>&, b2Filter*, entity_id)) (&physics_system::query_polygon))
			.def("push_away_from_walls", &physics_system::push_away_from_walls),

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
			,
			
			luabind::class_<renderer>("renderer")
			.def_readwrite("visibility_expansion", &renderer::visibility_expansion)
			.def_readwrite("max_visibility_expansion_distance", &renderer::max_visibility_expansion_distance)
			.def_readwrite("draw_steering_forces", &renderer::draw_steering_forces)
			.def_readwrite("draw_substeering_forces", &renderer::draw_substeering_forces)
			.def_readwrite("draw_velocities", &renderer::draw_velocities)
			.def_readwrite("draw_avoidance_info", &renderer::draw_avoidance_info)
			.def_readwrite("draw_wandering_info", &renderer::draw_wandering_info)
			.def_readwrite("draw_visibility", &renderer::draw_visibility)
			.def_readwrite("draw_weapon_info", &renderer::draw_weapon_info)
			.def_readwrite("debug_drawing", &renderer::debug_drawing)
			.def_readwrite("triangles", &renderer::triangles)
			.def("push_line", &renderer::push_line)
			.def("push_non_cleared_line", &renderer::push_non_cleared_line)
			.def("push_line_channel", &renderer::push_line_channel)
			.def("clear_channel", &renderer::clear_channel)
			.def("clear_non_cleared_lines", &renderer::clear_non_cleared_lines)
			.def("fullscreen_quad", &renderer::fullscreen_quad)

			.def("default_render", &renderer::default_render)
			.def("get_triangle_count", &renderer::get_triangle_count)
			.def("get_triangle", &renderer::get_triangle)
			.def("call_triangles", &renderer::call_triangles)
			.def("push_triangle", &renderer::push_triangle)
			.def("clear_triangles", &renderer::clear_triangles)
			.def("draw_debug_info", &renderer::draw_debug_info)
			,

			luabind::class_<render_system>("_render_system")
			.def("generate_layers", &render_system::generate_layers)
			.def("draw_layer", &render_system::draw_layer)
			.def("generate_and_draw_all_layers", &render_system::generate_and_draw_all_layers)
			,

			luabind::class_<input_system>("_input_system")
			.def("add_context", &input_system::add_context)
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
			luabind::class_<destroy_system>("_destroy_system"),
			luabind::class_<particle_group_system>("_particle_group_system")
			.def("process_entities", &particle_group_system::process_entities),
			luabind::class_<particle_emitter_system>("_particle_emitter_system")
			.def("consume_events", &particle_emitter_system::consume_events),

			luabind::def("create_refreshable_particle_group", &particle_emitter_system::create_refreshable_particle_group),

			bind_vector_wrapper<luabind::object>("luabindobject_vector"),
			
			luabind::class_<behaviour_tree_system>("_behaviour_tree_system")
			.def("process_entities", &behaviour_tree_system::process_entities)
			;
			
	}
}