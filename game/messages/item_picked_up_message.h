#pragma once
#include "message.h"

namespace messages {
	struct item_picked_up_message : message {
		entity_id item;
	};
}
