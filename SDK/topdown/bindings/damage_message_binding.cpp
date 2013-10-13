#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/damage_message.h"

#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _damage_message() {
		return (
			luabind::class_<damage_message>("damage_message")
			.def(luabind::constructor<>())
			.def_readwrite("subject", &damage_message::subject)
			.def_readwrite("amount", &damage_message::amount)
			.def_readwrite("impact_velocity", &damage_message::impact_velocity)
			);
	}
}