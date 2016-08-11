#pragma once

#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"

class position_copying_system;

namespace components {
	struct position_copying {
		entity_id target;
		
		enum position_copying_type {
			OFFSET,
			ORBIT,
			PARALLAX
		} position_copying_type = position_copying_type::OFFSET;

		vec2 offset;
		vec2 rotation_orbit_offset;
		
		vec2 reference_position;
		vec2 target_reference_position;
		
		float scrolling_speed = 1.0f;

		float rotation_offset = 0.0f;
		float rotation_multiplier = 1.0f;

		bool relative = false;
		bool position_copying_rotation = false;
		bool track_origin = false;

		bool target_newly_set = true;
		bool subscribe_to_previous = false;

	private:
		friend class position_copying_system;

		vec2 previous;
		float rotation_previous = 0.0f;

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
				CEREAL_NVP(rotation_previous)
			);
		}

		position_copying(entity_id id = entity_id()) {
			set_target(id);
		}

		void set_target(entity_id);
	};
}