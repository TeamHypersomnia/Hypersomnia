#pragma once

#include "entity_system/entity_id.h"
#include "../shared/drawing_state.h"

namespace messages {
	struct camera_render_request_message {
		augs::entity_id camera;
		shared::drawing_state state; 
		int mask;
	};
}