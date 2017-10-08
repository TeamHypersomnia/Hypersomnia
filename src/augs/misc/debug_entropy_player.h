#pragma once
#include <fstream>
#include <string>
#include <map>

#include "augs/misc/time_utils.h"
#include "augs/readwrite/byte_readwrite.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

namespace augs {
	template <class entropy_type>
	class debug_entropy_player {
		enum class player_state {
			DISABLED,
			RECORDING,
			REPLAYING
		};

		player_state current_player_state = player_state::DISABLED;

		std::map<unsigned, entropy_type> step_to_entropy_to_replay;
		unsigned player_step_position = 0u;

		path_type live_saving_path;

	public:
		void advance_player_and_biserialize(entropy_type& total_collected_entropy) {
			if (is_replaying()) {
				if (player_step_position > (*step_to_entropy_to_replay.rbegin()).first) {
					current_player_state = player_state::DISABLED;
				}

				const auto& recording = step_to_entropy_to_replay;
				const auto it = recording.find(player_step_position);

				if (it != recording.end()) {
					total_collected_entropy = (*it).second;
				}
				else {
					total_collected_entropy = entropy_type();
				}

				++player_step_position;
			}
			else if (is_recording()) {
				if (total_collected_entropy.length() > 0) {
					auto recording_file = with_exceptions<std::ofstream>();
					recording_file.open(live_saving_path, std::ios::out | std::ios::binary | std::ios::app);

					augs::write(recording_file, player_step_position);
					augs::write(recording_file, total_collected_entropy);
				}

				++player_step_position;
			}
		}

		bool try_to_load_and_replay_recording(const path_type& path) {
			player_step_position = 0;
			step_to_entropy_to_replay.clear();

			read_map_until_eof(path, step_to_entropy_to_replay);

			if (step_to_entropy_to_replay.empty()) {
				return false;
			}
			else {
				current_player_state = player_state::REPLAYING;
				return true;
			}
		}

		void record_and_save_this_session(const path_type& folder, const path_type& path) {
			const auto target_folder = path_type(folder.string() + augs::get_timestamp() + "/");
			augs::create_directories(target_folder);

			live_saving_path = target_folder.string() + "/" + path.string();
			current_player_state = player_state::RECORDING;
		}

		bool try_to_load_or_save_new_session(const path_type& sessions_folder, const path_type& recording_path) {
			std::vector<path_type> paths = { recording_path };
			const auto recording_path_str = recording_path.string();

			if (recording_path_str.find(".inputs") != std::string::npos) {
				paths.push_back(recording_path_str.substr(0, recording_path_str.size() - std::string(".inputs").length()) + "_0.inputs");
				paths.push_back(recording_path_str.substr(0, recording_path_str.size() - std::string(".inputs").length()) + "_1.inputs");
			}

			for (const auto& path : paths) {
				if (try_to_load_and_replay_recording(path)) {
					return true;
				}
			}

			record_and_save_this_session(sessions_folder, recording_path);
			return false;
		}

		bool is_replaying() const {
			return current_player_state == player_state::REPLAYING;
		}

		bool is_recording() const {
			return current_player_state == player_state::RECORDING;
		}
	};
}