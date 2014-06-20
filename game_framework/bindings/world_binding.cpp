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
std::vector<T> get_message_queue_for_scripts(world& owner) {
	std::vector<T> msgs = owner.get_message_queue<T>();
	
	msgs.erase(std::remove_if(msgs.begin(), msgs.end(), [](const T& msg){
		return !msg.send_to_scripts;
	}), msgs.end());

	return msgs;
}

namespace bindings {
	luabind::scope _world() {
		return
			augs::misc::vector_wrapper<destroy_message>::bind_vector("destroy_message_vector"),
			augs::misc::vector_wrapper<collision_message>::bind_vector("collision_message_vector"),
			augs::misc::vector_wrapper<damage_message>::bind_vector("damage_message_vector"),
			augs::misc::vector_wrapper<intent_message>::bind_vector("intent_message_vector"),
			augs::misc::vector_wrapper<shot_message>::bind_vector("shot_message_vector"),


			luabind::class_<world>("_world")
			.def(luabind::constructor<>())
			.def("validate_delayed_messages", &world::validate_delayed_messages)
			.def("flush_message_queues", &world::flush_message_queues)
			.def("create_entity", &world::create_entity)
			.def("delete_entity", &world::delete_entity)
			.def("delete_all_entities", &world::delete_all_entities)
			.def("post_message", &world::post_message<animate_message>)
			.def("post_message", &world::post_message<intent_message>)
			.def("post_message", &world::post_message<destroy_message>)
			.def("post_message", &world::post_message<particle_burst_message>)


			.def("post_delayed_message", &world::post_delayed_message<animate_message>)
			.def("post_delayed_message", &world::post_delayed_message<intent_message>)
			.def("post_delayed_message", &world::post_delayed_message<destroy_message>)
			.def("post_delayed_message", &world::post_delayed_message<particle_burst_message>)
			,

			luabind::def("get_destroy_message_queue", get_message_queue_for_scripts<destroy_message>),
			luabind::def("get_collision_message_queue", get_message_queue_for_scripts<collision_message>),
			luabind::def("get_damage_message_queue", get_message_queue_for_scripts<damage_message>),
			luabind::def("get_intent_message_queue", get_message_queue_for_scripts<intent_message>),
			luabind::def("get_shot_message_queue", get_message_queue_for_scripts<shot_message>);
	}
}