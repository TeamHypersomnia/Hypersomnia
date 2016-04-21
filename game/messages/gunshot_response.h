#pragma once
#include "message.h"
#include "../components/transform_component.h"

namespace messages {
	struct gunshot_response : message {
		components::transform barrel_transform;
		std::vector<augs::entity_id> spawned_rounds;
		std::vector<augs::entity_id> spawned_shells;
	};
}
