#pragma once
#include "game/transcendental/entity_id.h"
#include "padding_byte.h"

namespace components {
	struct trigger {
		entity_id entity_to_be_notified;
		bool react_to_collision_detectors = false;
		bool react_to_query_detectors = true;
		std::array<padding_byte, 2> pad;

		template<class F>
		void for_each_held_id(F f) {
			f(entity_to_be_notified);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(entity_to_be_notified);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(entity_to_be_notified),
				CEREAL_NVP(react_to_collision_detectors),
				CEREAL_NVP(react_to_query_detectors)
			);
		}
	};
}
