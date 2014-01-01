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
			luabind::class_<world>("world")
			.def(luabind::constructor<>())
			.def("create_entity", &world::create_entity)
			.def("delete_entity", &world::delete_entity)
			.def("post_message", &world::post_message<animate_message>)
			.def("post_message", &world::post_message<intent_message>)
			.def("post_destroy_message", &world::post_message<destroy_message>)
			.def("post_message", &world::post_message<particle_burst_message>)
			,

			luabind::def("post_destroy", helper_destroy);
	}
}