#pragma once
#include "augs/math/camera_cone.h"
#include "augs/window_framework/event.h"

struct debugger_view;

struct editor_camera_settings {
	float panning_speed = 1.0f;
};

namespace editor_detail {
	bool handle_camera_input(
		const editor_camera_settings& settings,
		camera_cone current_cone,
		const augs::event::state& common_input_state,
		const augs::event::change e,
		vec2 world_cursor_pos,
		camera_eye& panned_camera
	);
}
