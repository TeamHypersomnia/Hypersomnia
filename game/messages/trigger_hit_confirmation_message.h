#pragma once
#include "entity_system/entity_id.h"

namespace messages {
	struct trigger_hit_confirmation_message {
		augs::entity_id detector_body;
		augs::entity_id trigger;
	};
}