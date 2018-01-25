#pragma once
#include <vector>
#include "game/messages/message.h"

namespace messages {
	struct queue_destruction : public message {
		queue_destruction(entity_id s) : message(s) {}
	};
}

using destruction_queue = std::vector<messages::queue_destruction>;