#pragma once
#include "message.h"
#include "augs/math/vec2.h"
#include "game/detail/physics/b2Fixture_index_in_component.h"

namespace messages {
	struct damage_message : public message {
		bool inflictor_destructed = false;
		float amount = 0.f;
		entity_id inflictor;
		vec2 impact_velocity;
		vec2 point_of_impact;

		b2Fixture_index_in_component collider_b2Fixture_index;
		b2Fixture_index_in_component subject_b2Fixture_index;
	};
}