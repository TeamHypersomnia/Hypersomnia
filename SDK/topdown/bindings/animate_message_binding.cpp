#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/animate_info.h"
#include "../messages/animate_message.h"

#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _animate_message() {
		struct dummy_animation {};
		return (
			luabind::class_<animate_message>("animate_message")
			.def(luabind::constructor<>())
			.def(luabind::constructor<animate_message::animation, animate_message::type, bool, float>())
			.def_readwrite("subject", &animate_message::subject)
			.def_readwrite("set_animation", &animate_message::set_animation)
			.def_readwrite("animation_type", &animate_message::animation_type)
			.def_readwrite("preserve_state_if_animation_changes", &animate_message::preserve_state_if_animation_changes)
			.def_readwrite("message_type", &animate_message::message_type)
			.def_readwrite("change_animation", &animate_message::change_animation)
			.def_readwrite("change_speed", &animate_message::change_speed)
			.def_readwrite("speed_factor", &animate_message::speed_factor)
			.def_readwrite("animation_priority", &animate_message::animation_priority)
			.enum_("constants")
			[
				luabind::value("CONTINUE", animate_message::CONTINUE),
				luabind::value("START", animate_message::START),
				luabind::value("PAUSE", animate_message::PAUSE),
				luabind::value("STOP", animate_message::STOP)
			],

			luabind::class_<dummy_animation>("animation_events")
			.enum_("constants")
			[
				luabind::value("MOVE", animate_message::MOVE),
				luabind::value("SWING_CW", animate_message::SWING_CW),
				luabind::value("SWING_CCW", animate_message::SWING_CCW),
				luabind::value("SHOT", animate_message::SHOT)
			]
				);
	}
}