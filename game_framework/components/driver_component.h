#pragma once
#include "entity_system/entity.h"

namespace components {
	struct driver {
		augs::entity_id owned_vehicle;
		float density_multiplier_while_driving = 1.f/3.f;
	};
}