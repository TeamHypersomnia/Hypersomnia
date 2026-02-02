#pragma once
#include "game/messages/message.h"
#include "augs/math/vec2.h"
#include "game/detail/physics/b2Fixture_index_in_component.h"
#include "game/detail/sentience_shake.h"
#include "game/enums/adverse_element_type.h"
#include "augs/misc/value_meter.h"
#include "game/detail/damage_origin.h"
#include "game/detail/damage/damage_definition.h"

namespace messages {
	struct damage_message : public message {
		bool spawn_destruction_effects = false;
		bool processed = false;
		bool from_pending_destruction = false;
		transformr head_transform;
		real32 headshot_mult = 1.0f;

		damage_definition damage;
		damage_origin origin;
		vec2 impact_velocity;
		vec2 point_of_impact;
		vec2 normal = vec2::zero;
		adverse_element_type type = adverse_element_type::FORCE;

		b2Fixture_indices indices;
	};
}