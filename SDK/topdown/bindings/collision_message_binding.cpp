#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/collision_message.h"

#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _collision_message() {
		return (
			luabind::class_<collision_message>("collision_message")
			.def(luabind::constructor<>())
			.def_readwrite("subject", &collision_message::subject)
			.def_readwrite("collider", &collision_message::collider)
			.def_readwrite("impact_velocity", &collision_message::impact_velocity)
			.def_readwrite("point", &collision_message::point)
			.def_readwrite("sensor_end_contact", &collision_message::sensor_end_contact)
		);
	}
}