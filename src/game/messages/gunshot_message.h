#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"
#include "game/components/cartridge_component.h"

namespace messages {
	struct gunshot_message : message {
		transformr muzzle_transform;

		std::vector<entity_id> spawned_rounds;
		entity_id capability;
	};
}
