#pragma once
#include "game/entity_id.h"
#include "game/detail/state_for_drawing_camera.h"

namespace messages {
	struct camera_render_request_message {
		entity_id camera;
		shared::state_for_drawing_camera state;

		vec2 get_screen_space(vec2 pos) {
			return pos - state.transformed_visible_world_area_aabb.get_position();
		}
		int mask;
	};
}