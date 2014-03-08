#pragma once
#include "stdafx.h"
#include "entity_system/world.h"
#include "bindings.h"

#include "../messages/animate_message.h"
#include "../messages/intent_message.h"
#include "../messages/destroy_message.h"
#include "../messages/particle_burst_message.h"


void helper_destroy(world& owner, destroy_message msg) {
	owner.post_message(msg);
}

namespace bindings {
	luabind::scope _world() {
		return
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

			luabind::def("post_destroy", helper_destroy);
	}
}