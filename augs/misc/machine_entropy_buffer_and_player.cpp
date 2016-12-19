#include "augs/misc/templated_readwrite.h"
#include "machine_entropy_buffer_and_player.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"
#include "game/transcendental/cosmos.h"

namespace augs {
	void machine_entropy_buffer_and_player::advance_player_and_biserialize(machine_entropy& total_collected_entropy) {
		if (is_replaying()) {
			const auto& recording = step_to_entropy_to_replay;
			const auto it = recording.find(player_step_position);

			if (it != recording.end()) {
				total_collected_entropy = (*it).second;
			}

			++player_step_position;
		}
		else if (is_recording()) {
			if (!total_collected_entropy.empty()) {
				std::ofstream recording_file(live_saving_filename, std::ios::out | std::ios::binary | std::ios::app);

				augs::write_object(recording_file, player_step_position);
				augs::write_object(recording_file, total_collected_entropy);
			}

			++player_step_position;
		}
	}

	bool machine_entropy_buffer_and_player::try_to_load_and_replay_recording(const std::string& filename) {
		player_step_position = 0;
		step_to_entropy_to_replay.clear();
		
		std::ifstream source(filename, std::ios::in | std::ios::binary);

		while (source.peek() != EOF) {
			unsigned step;
			augs::machine_entropy ent;

			augs::read_object(source, step);
			augs::read_object(source, ent);

			step_to_entropy_to_replay.emplace(step, std::move(ent));
		}
		
		if (step_to_entropy_to_replay.empty()) {
			return false;
		}
		else {
			current_player_state = player_state::REPLAYING;
			return true;
		}
	}

	void machine_entropy_buffer_and_player::record_and_save_this_session(const std::string& folder, const std::string& filename) {
		const auto target_folder = folder + augs::get_timestamp();
		augs::create_directories(target_folder);

		live_saving_filename = target_folder + "/" + filename;
		current_player_state = player_state::RECORDING;
	}

	bool machine_entropy_buffer_and_player::try_to_load_or_save_new_session(const std::string& sessions_folder, const std::string& recording_filename) {
		std::vector<std::string> filenames = { recording_filename };

		if (recording_filename.find(".inputs") != std::string::npos) {
			filenames.push_back(recording_filename.substr(0, recording_filename.size() - std::string(".inputs").length()) + "_0.inputs");
			filenames.push_back(recording_filename.substr(0, recording_filename.size() - std::string(".inputs").length()) + "_1.inputs");
		}

		for (const auto& filename : filenames) {
			if (try_to_load_and_replay_recording(filename)) {
				return true;
			}
		}

		record_and_save_this_session(sessions_folder, recording_filename);
		return false;
	}

	bool machine_entropy_buffer_and_player::is_replaying() const {
		return current_player_state == player_state::REPLAYING;
	}

	bool machine_entropy_buffer_and_player::is_recording() const {
		return current_player_state == player_state::RECORDING;
	}
}