#pragma once
#include "message.h"

namespace messages {
	struct queue_destruction : public message {
		queue_destruction(entity_id s) : message(s) {}
	};
}