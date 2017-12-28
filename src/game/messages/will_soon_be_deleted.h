#pragma once
#include <vector>
#include "message.h"

namespace messages {
	struct will_soon_be_deleted : message {
		will_soon_be_deleted(const entity_id s) : message(s) {}
	};
}

using deletion_queue = std::vector<messages::will_soon_be_deleted>;
