#pragma once
#include <unordered_map>
#include "augs/misc/machine_entropy.h"
#include "game/transcendental/cosmic_entropy.h"

namespace augs {
	class machine_entropy_buffer_and_player {
		enum class player_state {
			DISABLED,
			RECORDING,
			REPLAYING
		};

		player_state current_player_state = player_state::DISABLED;

		std::unordered_map<unsigned, machine_entropy> step_to_entropy_to_replay;
		unsigned player_step_position = 0u;

		std::string live_saving_filename;

	public:
		void advance_player_and_biserialize(machine_entropy&);

		void record_and_save_this_session(const std::string& folder, const std::string& filename);
		bool try_to_load_and_replay_recording(const std::string& filename);

		bool try_to_load_or_save_new_session(const std::string& sessions_folder, const std::string& recording_filename);

		bool is_replaying() const;
		bool is_recording() const;
	};
}