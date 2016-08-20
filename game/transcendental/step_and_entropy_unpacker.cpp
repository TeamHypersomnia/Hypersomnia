#include "step_and_entropy_unpacker.h"

void step_and_entropy_unpacker::control(const augs::machine_entropy& entropy) {
	player.buffer_entropy_for_next_step(entropy);
}

std::vector<step_and_entropy_unpacker::simulation_step> step_and_entropy_unpacker::unpack_steps(const augs::fixed_delta& dt) {
	std::vector<step_and_entropy_unpacker::simulation_step> output;
	
	auto steps_to_perform = timer.count_logic_steps_to_perform(dt);

	while (steps_to_perform--)
		output.push_back({ player.obtain_machine_entropy_for_next_step() });

	return std::move(output);
}

bool step_and_entropy_unpacker::try_to_load_or_save_new_session(std::string sessions_folder, std::string recording_filename) {
	if (!player.try_to_load_and_replay_recording(recording_filename)) {
		player.record_and_save_this_session(sessions_folder, recording_filename);

		return false;
	}

	return true;
}