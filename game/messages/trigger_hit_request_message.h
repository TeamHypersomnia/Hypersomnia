#pragma once
#include "game/entity_id.h"

namespace messages {
	struct trigger_hit_request_message {
		entity_id detector;
	};
}