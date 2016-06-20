#pragma once
#include "stdafx.h"
#include "game/cosmos.h"
#include "game/bindings/bindings.h"

#include "game/stateful_systems/physics_system.h"
#include "game/systems/movement_system.h"
#include "game/systems/visibility_system.h"
#include "game/systems/pathfinding_system.h"
#include "game/systems/animation_system.h"
#include "game/systems/camera_system.h"
#include "game/systems/render_system.h"
#include "game/systems/input_system.h"
#include "game/systems/gun_system.h"
#include "game/systems/crosshair_system.h"
#include "game/systems/rotation_copying_system.h"
#include "game/systems/position_copying_system.h"
#include "game/systems/damage_system.h"
#include "game/systems/destroy_system.h"
#include "game/systems/particles_system.h"
#include "game/systems/behaviour_tree_system.h"
#include "game/systems/car_system.h"
#include "game/systems/driver_system.h"
#include "game/systems/trigger_detector_system.h"
#include "game/systems/item_system.h"
#include "game/systems/force_joint_system.h"
#include "game/systems/intent_contextualization_system.h"
#include "game/stateful_systems/gui_system.h"
#include "game/systems/trace_system.h"
#include "game/systems/melee_system.h"
#include "game/systems/sentience_system.h"
#include "game/stateful_systems/dynamic_tree_system.h"


#include "misc/vector_wrapper.h"


template <typename T>
std::vector<T>& get_message_queue_for_scripts(cosmos& owner) {
	return owner.messages.get_queue<T>();
}

namespace bindings {
	luabind::scope _world() {
		return luabind::scope();
			//bind_stdvector<queue_destruction>("destroy_message_vector"),
			//bind_stdvector<animation_message>("animation_message_vector"),
			//bind_stdvector<create_particle_effect>("particle_burst_message_vector"),
			//bind_stdvector<collision_message>("collision_message_vector"),
			//bind_stdvector<damage_message>("damage_message_vector"),
			//bind_stdvector<intent_message>("intent_message_vector"),
			//bind_stdvector<gunshot_response>("gunshot_response_vector"),

			//luabind::class_<world>("_world")
			//.def(luabind::constructor<overworld&>())
			//.def("create_entity", &world::create_entity)
			//.def("delete_entity", &world::delete_entity)
			//.def("delete_all_entities", &world::delete_all_entities)
			//.def("post_message", &world::post_message<animation_message>)
			//.def("post_message", &world::post_message<intent_message>)
			//.def("post_message", &world::post_message<queue_destruction>)
			//.def("post_message", &world::post_message<create_particle_effect>)
			//
			//.property("input_system", &world::systems.get<input_system>)
			//.property("steering_system", &world::systems.get<steering_system>)
			//.property("movement_system", &world::systems.get<movement_system>)
			//.property("animation_system", &world::systems.get<animation_system>)
			//.property("crosshair_system", &world::systems.get<crosshair_system>)
			//.property("rotation_copying_system", &world::systems.get<rotation_copying_system>)
			//.property("physics_system", &world::systems.get<physics_system>)
			//.property("visibility_system", &world::systems.get<visibility_system>)
			//.property("pathfinding_system", &world::systems.get<pathfinding_system>)
			//.property("gun_system", &world::systems.get<gun_system>)
			//.property("particles_system", &world::systems.get<particles_system>)
			//.property("render_system", &world::systems.get<render_system>)
			//.property("camera_system", &world::systems.get<camera_system>)
			//.property("position_copying_system", &world::systems.get<position_copying_system>)
			//.property("damage_system", &world::systems.get<damage_system>)
			//.property("destroy_system", &world::systems.get<destroy_system>)
			//.property("behaviour_tree_system", &world::systems.get<behaviour_tree_system>)
			//,
			//
			//luabind::def("get_destroy_message_queue", get_message_queue_for_scripts<queue_destruction>),
			//luabind::def("get_collision_message_queue", get_message_queue_for_scripts<collision_message>),
			//luabind::def("get_damage_message_queue", get_message_queue_for_scripts<damage_message>),
			//luabind::def("get_intent_message_queue", get_message_queue_for_scripts<intent_message>),
			//luabind::def("get_particle_burst_message_queue", get_message_queue_for_scripts<create_particle_effect>),
			//luabind::def("get_animation_message_queue", get_message_queue_for_scripts<animation_message>),
			//luabind::def("get_gunshot_response_queue", get_message_queue_for_scripts<gunshot_response>);
	}
}