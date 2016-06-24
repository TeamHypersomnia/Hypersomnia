#pragma once

class cosmos;
class fixed_step;

class animation_system {
public:
	void game_responses_to_animation_messages(fixed_step& step);

	void handle_animation_messages(fixed_step& step);
	void progress_animation_states(fixed_step& step);
};