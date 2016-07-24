#pragma once
#include "augs/math/vec2.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/timer.h"
#include "augs/graphics/pixel.h"

namespace components {
	struct damage {
		float amount = 12.f;

		float impulse_upon_hit = 100.f;

		entity_id sender;
		bool damage_upon_collision = true;
		bool destroy_upon_damage = true;
		int damage_charges_before_destruction = 1;

		vec2 custom_impact_velocity;

		bool constrain_lifetime = true;
		bool constrain_distance = false;
		bool damage_falloff = false;

		float damage_falloff_starting_distance = 500.f;
		float minimum_amount_after_falloff = 5.f;

		float distance_travelled = 0.f;
		float max_distance = 0.f;
		float max_lifetime_ms = 2000.f;
		float recoil_multiplier = 1.f;

		float lifetime_ms = 0.f;

		vec2 saved_point_of_impact_before_death;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(amount),

				CEREAL_NVP(impulse_upon_hit),

				CEREAL_NVP(sender),
				CEREAL_NVP(damage_upon_collision),
				CEREAL_NVP(destroy_upon_damage),
				CEREAL_NVP(damage_charges_before_destruction),

				CEREAL_NVP(custom_impact_velocity),

				CEREAL_NVP(constrain_lifetime),
				CEREAL_NVP(constrain_distance),
				CEREAL_NVP(damage_falloff),

				CEREAL_NVP(damage_falloff_starting_distance),
				CEREAL_NVP(minimum_amount_after_falloff),

				CEREAL_NVP(distance_travelled),
				CEREAL_NVP(max_distance),
				CEREAL_NVP(max_lifetime_ms),
				CEREAL_NVP(recoil_multiplier),

				CEREAL_NVP(lifetime_ms),

				CEREAL_NVP(saved_point_of_impact_before_death)
				);
		}

		static bool can_merge_entities(const_entity_handle a, const_entity_handle b);
	};
}