#pragma once

#include "entity_system/entity_id.h"

namespace messages {
	struct new_entity_message {
		augs::entity_id subject;
	};
}
