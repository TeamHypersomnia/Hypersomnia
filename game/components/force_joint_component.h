#pragma once
#include <array>
#include "transform_component.h"

#include "game/transcendental/entity_id.h"

#include "augs/math/vec2.h"

namespace components {
	struct force_joint {
		entity_id chased_entity;

		float force_towards_chased_entity = 8000.f;
		float distance_when_force_easing_starts = 10.f;
		float power_of_force_easing_multiplier = 2.f;

		float percent_applied_to_chased_entity = 0.f;

		bool divide_transform_mode = false;
		bool consider_rotation = true;
		components::transform chased_entity_offset;

		std::array<vec2, 2> force_offsets;

		template<class F>
		void for_each_held_id(F f) {
			f(chased_entity);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(chased_entity);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(chased_entity),

				CEREAL_NVP(force_towards_chased_entity),
				CEREAL_NVP(distance_when_force_easing_starts),
				CEREAL_NVP(power_of_force_easing_multiplier),

				CEREAL_NVP(percent_applied_to_chased_entity),

				CEREAL_NVP(divide_transform_mode),
				CEREAL_NVP(consider_rotation),
				CEREAL_NVP(chased_entity_offset),

				CEREAL_NVP(force_offsets)
			);
		}
	};
}