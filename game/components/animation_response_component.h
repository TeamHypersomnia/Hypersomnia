#pragma once
#include "game/assets/animation_response_id.h"

namespace components {
	struct animation_response {
		assets::animation_response_id response = assets::animation_response_id::INVALID;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(response));
		}
	};
}