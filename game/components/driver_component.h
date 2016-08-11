#pragma once
#include "game/transcendental/entity_id.h"

namespace components {
	struct driver {
		entity_id owned_vehicle;
		float density_multiplier_while_driving = 1.f/3.f;

		template<class F>
		void for_each_held_id(F f) {
			f(owned_vehicle);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(owned_vehicle);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(owned_vehicle),
				CEREAL_NVP(density_multiplier_while_driving)
			);
		}
	};
}