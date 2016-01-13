#pragma once
#include "entity_system/entity_id.h"

namespace messages {
	struct car_ownership_change_message {
		bool lost_ownership = false;
		augs::entity_id car;
		augs::entity_id driver;
	};
}