#pragma once
#include "math/vec2.h"

#include "entity_system/entity.h"

#include "misc/timer.h"
#include "graphics/pixel.h"

namespace components {
	struct damage {
		float amount = 12.f;

		float impulse_upon_hit = 100.f;

		augs::entity_id sender;
		bool damage_upon_collision = true;
		bool destroy_upon_damage = true;
		int damage_charges_before_destruction = 1;

		/* used to destroy bullets */
		vec2 starting_point;

		vec2 custom_impact_velocity;

		bool constrain_lifetime = true;
		bool constrain_distance = false;

		float max_distance = 0.f;
		float max_lifetime_ms = 2000.f;
		
		float lifetime_ms = 0.f;

		vec2 saved_point_of_impact_before_death;

		static bool can_merge_entities(augs::entity_id a, augs::entity_id b);
	};
}