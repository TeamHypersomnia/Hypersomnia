#pragma once
#include "game/entity_id.h"

namespace components {
	struct trigger {
		entity_id entity_to_be_notified;
		bool react_to_collision_detectors = false;
		bool react_to_query_detectors = true;
	};
}
