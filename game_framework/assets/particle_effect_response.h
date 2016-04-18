#pragma once
#include "../resources/particle_effect_response.h"

namespace assets {
	enum particle_effect_response_id {
		FIREARM_RESPONSE,
		ROUND_RESPONSE,
	};
}

resources::particle_effect_response& operator*(const assets::particle_effect_response_id& id);
