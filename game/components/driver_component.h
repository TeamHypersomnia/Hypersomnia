#pragma once
#include "game/transcendental/entity_id.h"

namespace components {
	struct driver {
		entity_id owned_vehicle;
		float density_multiplier_while_driving = 1.f/3.f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(owned_vehicle),
				CEREAL_NVP(density_multiplier_while_driving)
			);
		}
	};
}