#pragma once
#include "../components/physics_definition_component.h"

namespace messages {
	struct rebuild_physics_message {
		augs::entity_id subject;
		components::physics_definition new_definition;
	};
}