#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"
#include "game/components/cartridge_component.h"

namespace messages {
	struct gunshot_response : message {
		components::transform muzzle_transform;
		invariants::cartridge cartridge_definition;

		std::vector<entity_id> spawned_rounds;
		entity_id spawned_shell;
	};
}
