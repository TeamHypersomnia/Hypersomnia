#pragma once
#include "message.h"

namespace messages {
	struct will_soon_be_deleted : message {
		will_soon_be_deleted(const entity_id s) : message(s) {}
	};
}