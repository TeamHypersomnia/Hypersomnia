#pragma once
#include "message.h"
#include "game/components/transform_component.h"

namespace messages {
	struct gunshot_response : message {
		components::transform muzzle_transform;
		std::vector<entity_id> spawned_rounds;
		std::vector<entity_id> spawned_shells;
	};
}
