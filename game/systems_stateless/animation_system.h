#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class animation_system {
public:
	void game_responses_to_animation_messages(const logic_step step);

	void handle_animation_messages(const logic_step step);
	void progress_animation_states(const logic_step step);
};