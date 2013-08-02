#pragma once
#include "../../../entity_system/entity_system.h"
#include "../../../utility/sorted_vector.h"
#include <algorithm>

namespace components {
	struct input : public augmentations::entity_system::component {
		augmentations::util::sorted_vector<unsigned> registered_events;
	};
}



