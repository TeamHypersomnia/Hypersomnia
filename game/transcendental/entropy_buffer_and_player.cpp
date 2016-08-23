#include "augs/misc/templated_readwrite.h"
#include "entropy_buffer_and_player.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"
#include "game/transcendental/cosmos.h"


void entropy_buffer_and_player::buffer_entropy_for_next_step(augs::machine_entropy delta) {
	total_buffered_entropy += delta;
}

augs::machine_entropy entropy_buffer_and_player::obtain_machine_entropy_for_next_step() {
	entropy_player.biserialize(total_buffered_entropy);
	
	auto resultant_entropy = total_buffered_entropy;
	total_buffered_entropy = augs::machine_entropy();

	return resultant_entropy;
}

bool entropy_buffer_and_player::try_to_load_and_replay_recording(std::string filename) {
	if (augs::file_exists(filename)) {
		entropy_player.load_recording(filename);
		entropy_player.replay();
	}

	return entropy_player.is_recording_available();
}

void entropy_buffer_and_player::record_and_save_this_session(std::string folder, std::string filename) {
	auto target_folder = folder + augs::get_timestamp();
	augs::create_directories(target_folder);

	entropy_player.record(target_folder + "/" + filename);
}

bool entropy_buffer_and_player::is_replaying() const {
	return entropy_player.is_replaying();
}
