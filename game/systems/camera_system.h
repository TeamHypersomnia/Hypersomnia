#pragma once

class cosmos;
class step_state;

class camera_system {
public:

	void react_to_input_intents(cosmos& cosmos, step_state& step);

	void resolve_cameras_transforms_and_smoothing(cosmos& cosmos, step_state& step);
	void post_render_requests_for_all_cameras(const cosmos& cosmos, step_state& step);
};