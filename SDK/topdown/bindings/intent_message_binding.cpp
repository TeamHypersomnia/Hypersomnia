#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/intent_message.h"
#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _intent_message() {
		return
			luabind::class_<intent_message>("intent_message")
			.def(luabind::constructor<>())
			.def_readwrite("subject", &intent_message::subject)
			.def_readwrite("intent", &intent_message::intent)
			.def_readwrite("state_flag", &intent_message::state_flag)
			.def_readwrite("mouse_pos", &intent_message::mouse_pos)
			.def_readwrite("mouse_rel", &intent_message::mouse_rel)
			.def_readwrite("wheel_amount", &intent_message::wheel_amount)
			.enum_("intent_type")[
					luabind::value("MOVE_FORWARD", intent_message::intent_type::MOVE_FORWARD),
					luabind::value("MOVE_BACKWARD", intent_message::intent_type::MOVE_BACKWARD),
					luabind::value("MOVE_LEFT", intent_message::intent_type::MOVE_LEFT),
					luabind::value("MOVE_RIGHT", intent_message::intent_type::MOVE_RIGHT),
					luabind::value("SHOOT", intent_message::intent_type::SHOOT),
					luabind::value("AIM", intent_message::intent_type::AIM),
					luabind::value("SWITCH_LOOK", intent_message::intent_type::SWITCH_LOOK),
					luabind::value("SWITCH_WEAPON", intent_message::intent_type::SWITCH_WEAPON)
			];
	}
}