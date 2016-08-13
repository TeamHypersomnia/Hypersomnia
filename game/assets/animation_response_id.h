#pragma once
#include "game/resources/animation_response.h"

namespace assets {
	enum class animation_response_id {
		INVALID,

		TORSO_SET,
	};
}

resources::animation_response& operator*(const assets::animation_response_id& id);
