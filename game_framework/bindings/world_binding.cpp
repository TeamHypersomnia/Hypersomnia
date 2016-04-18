#pragma once
#include "stdafx.h"
#include "entity_system/world.h"
#include "bindings.h"

#include "game/all_message_includes.h"
#include "game/all_system_includes.h"

#include "misc/vector_wrapper.h"


template <typename T>
std::vector<T>& get_message_queue_for_scripts(world& owner) {
	return owner.get_message_queue<T>();
}

namespace bindings {
	luabind::scope _world() {
		return
			bind_stdvector<destroy_message>("destroy_message_vector"),
			bind_stdvector<animation_message>("animation_message_vector"),
			bind_stdvector<create_particle_effect>("particle_burst_message_vector"),
			bind_stdvector<collision_message>("collision_message_vector"),
			bind_stdvector<damage_message>("damage_message_vector"),
			bind_stdvector<intent_message>("intent_message_vector"),
			bind_stdvector<gunshot_response>("gunshot_response_vector"),


			luabind::class_<world>("_world")
			.def(luabind::constructor<overworld&>())
			.def("create_entity", &world::create_entity)
			.def("delete_entity", &world::delete_entity)
			.def("delete_all_entities", &world::delete_all_entities)
			.def("post_message", &world::post_message<animation_message>)
			.def("post_message", &world::post_message<intent_message>)
			.def("post_message", &world::post_message<destroy_message>)
			.def("post_message", &world::post_message<create_particle_effect>)
			
			.property("input_system", &world::get_system<input_system>)
			.property("steering_system", &world::get_system<steering_system>)
			.property("movement_system", &world::get_system<movement_system>)
			.property("animation_system", &world::get_system<animation_system>)
			.property("crosshair_system", &world::get_system<crosshair_system>)
			.property("rotation_copying_system", &world::get_system<rotation_copying_system>)
			.property("physics_system", &world::get_system<physics_system>)
			.property("visibility_system", &world::get_system<visibility_system>)
			.property("pathfinding_system", &world::get_system<pathfinding_system>)
			.property("gun_system", &world::get_system<gun_system>)
			.property("particles_system", &world::get_system<particles_system>)
			.property("render_system", &world::get_system<render_system>)
			.property("camera_system", &world::get_system<camera_system>)
			.property("position_copying_system", &world::get_system<position_copying_system>)
			.property("damage_system", &world::get_system<damage_system>)
			.property("destroy_system", &world::get_system<destroy_system>)
			.property("behaviour_tree_system", &world::get_system<behaviour_tree_system>)
			,

			luabind::def("get_destroy_message_queue", get_message_queue_for_scripts<destroy_message>),
			luabind::def("get_collision_message_queue", get_message_queue_for_scripts<collision_message>),
			luabind::def("get_damage_message_queue", get_message_queue_for_scripts<damage_message>),
			luabind::def("get_intent_message_queue", get_message_queue_for_scripts<intent_message>),
			luabind::def("get_particle_burst_message_queue", get_message_queue_for_scripts<create_particle_effect>),
			luabind::def("get_animation_message_queue", get_message_queue_for_scripts<animation_message>),
			luabind::def("get_gunshot_response_queue", get_message_queue_for_scripts<gunshot_response>);
	}
}