#pragma once
#include "game/resources/animation_response.h"

namespace assets {
	enum class animation_response_id {
		INVALID,

		TORSO_SET,
		VIOLET_TORSO_SET,
		BLUE_TORSO_SET,
		COUNT
	};
}

resources::animation_response& operator*(const assets::animation_response_id& id);
