#pragma once
#include "entity_system/entity.h"

namespace components {
	struct car {
		augs::entity_id left_driver;
		augs::entity_id right_driver;


	};
}