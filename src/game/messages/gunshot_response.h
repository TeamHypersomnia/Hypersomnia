#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"
#include "game/components/catridge_component.h"

namespace messages {
	struct gunshot_response : message {
		components::transform muzzle_transform;
		components::catridge catridge_definition;

		std::vector<entity_id> spawned_rounds;
		entity_id spawned_shell;
	};
}
