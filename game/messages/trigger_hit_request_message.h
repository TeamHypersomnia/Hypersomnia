#pragma once
#include "game/transcendental/entity_id.h"

namespace messages {
	struct trigger_hit_request_message {
		entity_id detector;
	};
}