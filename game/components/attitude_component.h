#pragma once
#include <set>
#include "game/transcendental/entity_id.h"
#include "game/enums/attitude_type.h"

#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "padding_byte.h"

namespace components {
	struct attitude {
		float maximum_divergence_angle_before_shooting = 10.0;

		unsigned parties = 0;
		unsigned hostile_parties = 0;

		augs::constant_size_vector<entity_id, SPECIFIC_HOSTILE_ENTITIES_COUNT> specific_hostile_entities;
		
		entity_id currently_attacked_visible_entity;
		attitude_type target_attitude = attitude_type::NEUTRAL;

		bool is_alert = false;
		bool last_seen_target_position_inspected = false;
		padding_byte pad[2];

		vec2 last_seen_target_position;
		vec2 last_seen_target_velocity;

		template<class F>
		void for_each_held_id(F f) {
			f(currently_attacked_visible_entity);

			for (auto& e : specific_hostile_entities)
				f(e);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(currently_attacked_visible_entity);

			for (const auto& e : specific_hostile_entities)
				f(e);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(parties),
				CEREAL_NVP(hostile_parties),
				CEREAL_NVP(specific_hostile_entities),

				CEREAL_NVP(currently_attacked_visible_entity),
				CEREAL_NVP(target_attitude),

				CEREAL_NVP(is_alert),

				CEREAL_NVP(last_seen_target_position),
				CEREAL_NVP(last_seen_target_velocity),
				CEREAL_NVP(last_seen_target_position_inspected),
				
				CEREAL_NVP(maximum_divergence_angle_before_shooting)
			);
		}
	};
}
