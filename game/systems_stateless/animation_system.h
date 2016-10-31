#pragma once

class cosmos;
class logic_step;

class animation_system {
public:
	void game_responses_to_animation_messages(logic_step& step);

	void handle_animation_messages(logic_step& step);
	void progress_animation_states(logic_step& step);
};