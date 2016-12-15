#include "step_and_entropy_unpacker.h"

void step_and_entropy_unpacker::control(const augs::machine_entropy& entropy) {
	player.buffer_entropy_for_next_step(entropy);
}

std::vector<step_and_entropy_unpacker::simulation_step> step_and_entropy_unpacker::unpack_steps(const augs::fixed_delta& dt) {
	std::vector<step_and_entropy_unpacker::simulation_step> output;
	
	auto steps_to_perform = timer.count_logic_steps_to_perform(dt);

	while (steps_to_perform--) {
		output.push_back({ player.obtain_machine_entropy_for_next_step() });
	}

	return std::move(output);
}

bool step_and_entropy_unpacker::try_to_load_or_save_new_session(const input_recording_mode mode, std::string sessions_folder, std::string recording_filename) {
	std::vector<std::string> filenames = { recording_filename };

	if (recording_filename.find(".inputs") != std::string::npos) {
		filenames.push_back(recording_filename.substr(0, recording_filename.size() - std::string(".inputs").length()) + "_0.inputs");
		filenames.push_back(recording_filename.substr(0, recording_filename.size() - std::string(".inputs").length()) + "_1.inputs");
	}

	for (const auto& filename : filenames) {
		if (player.try_to_load_and_replay_recording(filename)) {
			return true;
		}
	}

	player.record_and_save_this_session(mode, sessions_folder, recording_filename);
	return false;
}