#pragma once
#include "stdafx.h"
#include "entity_system/world.h"
#include "bindings.h"

#include "../messages/animate_message.h"
#include "../messages/intent_message.h"
#include "../messages/destroy_message.h"
#include "../messages/particle_burst_message.h"

#include "../messages/collision_message.h"
#include "../messages/damage_message.h"
#include "../messages/shot_message.h"

#include "misc/vector_wrapper.h"

template <typename T>
std::vector<T>& get_message_queue_for_scripts(world& owner) {
	return owner.get_message_queue<T>();
}

namespace bindings {
	luabind::scope _world() {
		return
			bind_stdvector<destroy_message>("destroy_message_vector"),
			bind_stdvector<animate_message>("animate_message_vector"),
			bind_stdvector<particle_burst_message>("particle_burst_message_vector"),
			bind_stdvector<collision_message>("collision_message_vector"),
			bind_stdvector<damage_message>("damage_message_vector"),
			bind_stdvector<intent_message>("intent_message_vector"),
			bind_stdvector<shot_message>("shot_message_vector"),


			luabind::class_<world>("_world")
			.def(luabind::constructor<>())
			.def("create_entity", &world::create_entity)
			.def("delete_entity", &world::delete_entity)
			.def("delete_all_entities", &world::delete_all_entities)
			.def("post_message", &world::post_message<animate_message>)
			.def("post_message", &world::post_message<intent_message>)
			.def("post_message", &world::post_message<destroy_message>)
			.def("post_message", &world::post_message<particle_burst_message>),

			luabind::def("get_destroy_message_queue", get_message_queue_for_scripts<destroy_message>),
			luabind::def("get_collision_message_queue", get_message_queue_for_scripts<collision_message>),
			luabind::def("get_damage_message_queue", get_message_queue_for_scripts<damage_message>),
			luabind::def("get_intent_message_queue", get_message_queue_for_scripts<intent_message>),
			luabind::def("get_particle_burst_message_queue", get_message_queue_for_scripts<particle_burst_message>),
			luabind::def("get_animate_message_queue", get_message_queue_for_scripts<animate_message>),
			luabind::def("get_shot_message_queue", get_message_queue_for_scripts<shot_message>);
	}
}