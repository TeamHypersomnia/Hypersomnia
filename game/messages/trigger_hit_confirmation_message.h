#pragma once
#include "game/entity_id.h"

namespace messages {
	struct trigger_hit_confirmation_message {
		entity_id detector_body;
		entity_id trigger;
	};
}