#pragma once
#include "entity_system/entity.h"

namespace components {
	struct driver {
		augs::entity_id owned_vehicle;

		float density_while_driving = 0.2f;
		float standard_density = 0.6f;
	};
}