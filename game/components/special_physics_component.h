#pragma once
#include <vector>

#include "game/transcendental/entity_id.h"
#include "augs/misc/stepped_timing.h"

namespace components {
	struct special_physics {
		entity_id owner_friction_ground;
		std::vector<entity_id> owner_friction_grounds;

		augs::stepped_timeout since_dropped;

		bool enable_angle_motor = false;

		float target_angle = 0.f;
		float angle_motor_force_multiplier = 1.f;

		float measured_carried_mass = 0.f;

		/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
		float air_resistance = 2.0f;
		// -1.f - the same as the air resistance
		float angular_air_resistance = 0;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(owner_friction_ground),
				CEREAL_NVP(owner_friction_grounds),

				CEREAL_NVP(since_dropped),

				CEREAL_NVP(enable_angle_motor),

				CEREAL_NVP(target_angle),
				CEREAL_NVP(angle_motor_force_multiplier),

				CEREAL_NVP(measured_carried_mass),
				CEREAL_NVP(air_resistance),
				CEREAL_NVP(angular_air_resistance)
			);
		}
	};
}