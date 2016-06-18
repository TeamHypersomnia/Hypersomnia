#pragma once

#include "game/entity_id.h"

namespace messages {
	struct new_entity_message {
		bool delete_this_message = false;
		entity_id subject;
	};
}
