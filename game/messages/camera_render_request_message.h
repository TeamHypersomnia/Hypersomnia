#pragma once
#include "entity_system/entity_id.h"
#include "../detail/state_for_drawing.h"

namespace messages {
	struct camera_render_request_message {
		augs::entity_id camera;
		shared::state_for_drawing_camera state;
		int mask;
	};
}