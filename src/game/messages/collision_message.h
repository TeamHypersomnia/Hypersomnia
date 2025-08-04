#pragma once
#include "game/messages/message.h"
#include "augs/math/vec2.h"
#include "game/detail/physics/b2Fixture_index_in_component.h"

namespace messages {
	struct collision_message : public message {
		entity_id collider;
		vec2 subject_impact_velocity;
		vec2 collider_impact_velocity;
		vec2 point;
		vec2 normal;

		b2Fixture_indices indices;
		
		real32 normal_impulse = 0.f;
		real32 tangent_impulse = 0.f;

		bool one_is_sensor = false;

		enum class event_type {
			BEGIN_CONTACT,
			PRE_SOLVE,
			POST_SOLVE,
			END_CONTACT
		} type = event_type::BEGIN_CONTACT;
	};
}