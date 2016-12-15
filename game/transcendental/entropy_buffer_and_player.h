#pragma once
#include "augs/misc/machine_entropy.h"
#include "game/transcendental/cosmic_entropy.h"

#include "augs/misc/step_player.h"
#include "game/enums/input_recording_mode.h"

class entropy_buffer_and_player {
	enum class player_state {
		DISABLED,
		RECORDING,
		REPLAYING
	};

	player_state current_player_state = player_state::DISABLED;

	augs::machine_entropy total_buffered_entropy;
	augs::step_player<augs::machine_entropy> machine_entropy_player;

public:
	void buffer_entropy_for_next_step(const augs::machine_entropy&);
	augs::machine_entropy obtain_machine_entropy_for_next_step();

	void record_and_save_this_session(const input_recording_mode, const std::string& folder, const std::string& filename);
	bool try_to_load_and_replay_recording(const std::string& filename);

	bool is_replaying() const;
	bool is_recording() const;
};