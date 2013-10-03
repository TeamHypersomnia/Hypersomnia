#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/intent_message.h"

namespace bindings {
	luabind::scope _intent_message() {
		return
			luabind::class_<intent_message>("intent_message")
			.def(luabind::constructor<>())
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