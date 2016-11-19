#pragma once
#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct interpolation {
		float base_exponent = 0.9f;
		components::transform place_of_birth;
	};
}