#pragma once
#include "game/transcendental/entropy_buffer_and_player.h"
#include "augs/misc/fixed_delta_timer.h"

class step_and_entropy_unpacker {
public:
	entropy_buffer_and_player player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	bool try_to_load_or_save_new_session(const input_recording_mode, std::string sessions_folder, std::string recording_filename);

	struct simulation_step {
		augs::machine_entropy total_entropy;
	};
	
	void control(const augs::machine_entropy&);

	std::vector<simulation_step> unpack_steps(const augs::fixed_delta&);
};