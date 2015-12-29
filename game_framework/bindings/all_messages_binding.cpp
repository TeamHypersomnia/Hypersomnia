#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/message.h"
#include "../all_message_includes.h"
#include "../resources/animate_info.h"

#include "entity_system/entity.h"

namespace bindings {
	struct dummy_animation {};

	luabind::scope _all_messages() {
		return
			luabind::class_<message>("message")
			.def(luabind::constructor<>())
			.def_readwrite("subject", &message::subject),
			
			luabind::class_<animate_message, message>("animate_message")
			.def(luabind::constructor<>())
			.def(luabind::constructor<animate_message::animation, animate_message::type, bool, float>())
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
			luabind::value("MOVE_CW", animate_message::MOVE_CW),
			luabind::value("MOVE_CCW", animate_message::MOVE_CCW),
			luabind::value("SHOT", animate_message::SHOT)
			],

			luabind::class_<collision_message, message>("collision_message")
			.def(luabind::constructor<>())
			.def_readwrite("collider", &collision_message::collider)
			.def_readwrite("subject_impact_velocity", &collision_message::subject_impact_velocity)
			.def_readwrite("collider_impact_velocity", &collision_message::collider_impact_velocity)
			.def_readwrite("point", &collision_message::point)
			.def_readwrite("sensor_end_contact", &collision_message::sensor_end_contact),

			luabind::class_<damage_message, message>("damage_message")
			.def(luabind::constructor<>())
			.def_readwrite("amount", &damage_message::amount)
			.def_readwrite("impact_velocity", &damage_message::impact_velocity),

			luabind::class_<destroy_message, message>("destroy_message")
			.def(luabind::constructor<>())
			.def(luabind::constructor<entity_id, entity_id>())
			.def_readwrite("redirection", &destroy_message::redirection)
			.def_readwrite("only_children", &destroy_message::only_children),

			luabind::class_<window::event::state>("event_state")
			.def(luabind::constructor<>())
			,

			luabind::class_<intent_message, message>("intent_message")
			.def(luabind::constructor<>())
			.def_readwrite("intent", &intent_message::intent)
			.def_readwrite("pressed_flag", &intent_message::pressed_flag)
			.def_readwrite("state", &intent_message::state)
			.enum_("intent_type")[
				luabind::value("MOVE_FORWARD", intent_message::intent_type::MOVE_FORWARD),
					luabind::value("MOVE_BACKWARD", intent_message::intent_type::MOVE_BACKWARD),
					luabind::value("MOVE_LEFT", intent_message::intent_type::MOVE_LEFT),
					luabind::value("MOVE_RIGHT", intent_message::intent_type::MOVE_RIGHT),
					luabind::value("SHOOT", intent_message::intent_type::SHOOT),
					luabind::value("AIM", intent_message::intent_type::AIM),
					luabind::value("SWITCH_LOOK", intent_message::intent_type::SWITCH_LOOK),
					luabind::value("SWITCH_WEAPON", intent_message::intent_type::SWITCH_WEAPON)
			],

			luabind::class_<particle_burst_message, message>("particle_burst_message")
					.def(luabind::constructor<>())
					.def_readwrite("pos", &particle_burst_message::pos)
					.def_readwrite("type", &particle_burst_message::type)
					.def_readwrite("rotation", &particle_burst_message::rotation)
					.def("set_effect", &particle_burst_message::set_effect)
					.def_readwrite("local_transform", &particle_burst_message::local_transform)
					.def_readwrite("target_group_to_refresh", &particle_burst_message::target_group_to_refresh)
					.enum_("burst_type")[
						luabind::value("BULLET_IMPACT", particle_burst_message::burst_type::BULLET_IMPACT),
							luabind::value("CUSTOM", particle_burst_message::burst_type::CUSTOM),
							luabind::value("WEAPON_SHOT", particle_burst_message::burst_type::WEAPON_SHOT)
					],


					luabind::class_<shot_message, message>("shot_message")
							.def(luabind::constructor<>())
			;
	}
}