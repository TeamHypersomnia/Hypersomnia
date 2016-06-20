#pragma once

class cosmos;
class step_state;

class animation_system {
public:
	void game_responses_to_animation_messages(cosmos& cosmos, step_state& step);

	void handle_animation_messages(cosmos& cosmos, step_state& step);
	void progress_animation_states(cosmos& cosmos, step_state& step);
};