#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "game/transcendental/entity_id.h"
#include "augs/misc/stepped_timing.h"

namespace components {
	struct special_physics {
		augs::stepped_timeout since_dropped;

		entity_id owner_friction_ground;

		struct friction_connection {
			friction_connection(entity_id t = entity_id()) : target(t) {}
			entity_id target;

			bool operator==(entity_id b) const {
				return target == b;
			}

			operator entity_id() const {
				return target;
			}

			unsigned fixtures_connected = 0;
		};

		augs::constant_size_vector<friction_connection, OWNER_FRICTION_GROUNDS_COUNT> owner_friction_grounds;

		int enable_angle_motor = false;

		float target_angle = 0.f;
		float angle_motor_force_multiplier = 1.f;

		float measured_carried_mass = 0.f;

		/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
		float air_resistance = 2.0f;
		// -1.f - the same as the air resistance
		float angular_air_resistance = 0;

		template<class F>
		void for_each_held_id(F f) {
			f(owner_friction_ground);

			for (auto& e : owner_friction_grounds)
				f(e.target);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(owner_friction_ground);

			for (const auto& e : owner_friction_grounds)
				f(e.target);
		}

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