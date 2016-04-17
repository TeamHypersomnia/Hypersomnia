#pragma once
#include "../resources/animation_response.h"

namespace assets {
	enum animation_response_id {
		TORSO_SET,
	};
}

resources::animation_response& operator*(const assets::animation_response_id& id);
