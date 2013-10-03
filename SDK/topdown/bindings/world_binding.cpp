#pragma once
#include "stdafx.h"
#include "entity_system/world.h"
#include "bindings.h"

#include "../messages/animate_message.h"
#include "../messages/intent_message.h"
#include "../messages/particle_burst_message.h"

namespace bindings {
	luabind::scope _world() {
		return
			luabind::class_<world>("world")
			.def(luabind::constructor<>())
			.def("create_entity", &world::create_entity)
			.def("delete_entity", &world::delete_entity)
			.def("post_message", &world::post_message<animate_message>)
			.def("post_message", &world::post_message<intent_message>)
			.def("post_message", &world::post_message<particle_burst_message>)
			;
	}
}