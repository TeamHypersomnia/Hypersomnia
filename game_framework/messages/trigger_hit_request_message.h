#pragma once
#include "entity_system/entity_id.h"

namespace messages {
	struct trigger_hit_request_message {
		augs::entity_id detector;
		bool pressed_flag = true;
	};
}