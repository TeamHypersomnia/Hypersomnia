#pragma once
#include <vector>
#include "game/messages/message.h"

namespace messages {
	struct queue_deletion : public message {
		queue_deletion(entity_id s) : message(s) {}
	};
}

using destruction_queue = std::vector<messages::queue_deletion>;