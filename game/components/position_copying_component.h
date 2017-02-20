#pragma once

#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"

class position_copying_system;

namespace components {
	struct position_copying {
		entity_id target;
		
		enum position_copying_type : unsigned char {
			OFFSET,
			ORBIT,
			PARALLAX
		};

		vec2 offset;
		vec2 rotation_orbit_offset;
		
		vec2 reference_position;
		vec2 target_reference_position;
		
		float scrolling_speed = 1.0f;

		float rotation_offset = 0.0f;
		float rotation_multiplier = 1.0f;

		position_copying_type position_copying_mode = position_copying_type::OFFSET;
		bool position_copying_rotation = false;
		bool track_origin = false;
		bool target_newly_set = true;

	private:
		friend class position_copying_system;

		components::transform previous;

	public:
		template<class F>
		void for_each_held_id(F f) {
			f(target);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(target);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(target),

				CEREAL_NVP(position_copying_type),

				CEREAL_NVP(offset),
				CEREAL_NVP(rotation_orbit_offset),

				CEREAL_NVP(reference_position),
				CEREAL_NVP(target_reference_position),

				CEREAL_NVP(scrolling_speed),

				CEREAL_NVP(rotation_offset),
				CEREAL_NVP(rotation_multiplier),

				CEREAL_NVP(relative),
				CEREAL_NVP(position_copying_rotation),
				CEREAL_NVP(track_origin),

				CEREAL_NVP(target_newly_set),
				CEREAL_NVP(subscribe_to_previous),

				CEREAL_NVP(previous),
			);
		}

		enum class chasing_configuration {
			NONE,
			RELATIVE_ORBIT
		};

		static position_copying configure_chasing(
			const const_entity_handle target,
			const components::transform chaser_place_of_birth,
			const chasing_configuration
		);

		void set_target(const const_entity_handle);
		components::transform get_previous_transform() const;
	};
}