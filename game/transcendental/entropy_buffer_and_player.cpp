#include "augs/misc/templated_readwrite.h"
#include "entropy_buffer_and_player.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"
#include "game/transcendental/cosmos.h"

void entropy_buffer_and_player::buffer_entropy_for_next_step(const augs::machine_entropy& delta) {
	if (is_recording()) {
		total_buffered_entropy += delta;
	}
}

augs::machine_entropy entropy_buffer_and_player::obtain_machine_entropy_for_next_step() {
	if (is_replaying()) {
		machine_entropy_player.replay_next_step(total_buffered_entropy);
	}
	else if (is_recording()) {
		machine_entropy_player.append_step_to_live_file(total_buffered_entropy);
	}

	const augs::machine_entropy resultant_entropy = std::move(total_buffered_entropy);
	total_buffered_entropy = augs::machine_entropy();

	return std::move(resultant_entropy);
}

bool entropy_buffer_and_player::try_to_load_and_replay_recording(const std::string& filename) {
	if (augs::file_exists(filename) && machine_entropy_player.load_recording(filename)) {
		current_player_state = player_state::REPLAYING;
		return true;
	}

	return false;
}

void entropy_buffer_and_player::record_and_save_this_session(const input_recording_mode mode, const std::string& folder, const std::string& filename) {
	const auto target_folder = folder + augs::get_timestamp();
	augs::create_directories(target_folder);

	if (mode != input_recording_mode::DISABLED) {
		machine_entropy_player.set_live_filename(target_folder + "/" + filename);
		current_player_state = player_state::RECORDING;
	}
}

bool entropy_buffer_and_player::is_replaying() const {
	return current_player_state == player_state::REPLAYING;
}

bool entropy_buffer_and_player::is_recording() const {
	return current_player_state == player_state::RECORDING;
}
