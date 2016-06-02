#pragma once

#include "entity_system/entity_id.h"

namespace messages {
	struct new_entity_message {
		bool delete_this_message = false;
		augs::entity_id subject;
	};
}
