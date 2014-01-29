#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/particle_burst_message.h"
#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _particle_burst_message() {
		return
			luabind::class_<particle_burst_message, message>("particle_burst_message")
			.def(luabind::constructor<>())
			.def_readwrite("pos", &particle_burst_message::pos)
			.def_readwrite("rotation", &particle_burst_message::rotation)
			.def_readwrite("set_effect", &particle_burst_message::set_effect)
			.def_readwrite("local_transform", &particle_burst_message::local_transform)
			.enum_("burst_type")[
				luabind::value("BULLET_IMPACT", particle_burst_message::burst_type::BULLET_IMPACT),
				luabind::value("WEAPON_SHOT", particle_burst_message::burst_type::WEAPON_SHOT)
			];
	}
}