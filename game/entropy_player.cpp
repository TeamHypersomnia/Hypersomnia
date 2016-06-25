#include "entropy_player.h"
#include "filesystem/file.h"
#include "filesystem/directory.h"
#include "misc/time.h"

void entropy_player::buffer_entropy_for_next_step(augs::machine_entropy delta) {
	total_buffered_entropy += delta;
}

augs::machine_entropy entropy_player::obtain_total_entropy_for_next_step() {
	local_entropy_player.biserialize(total_buffered_entropy.local);
	return total_buffered_entropy;
}

bool entropy_player::try_to_load_and_replay_recording(std::string filename) {
	if (augs::file_exists(filename)) {
		local_entropy_player.load_recording(filename);
		local_entropy_player.replay();
	}

	return local_entropy_player.is_recording_available();
}

void entropy_player::record_and_save_this_session(std::string folder, std::string filename) {
	augs::create_directory(folder);
	augs::create_directory(folder + augs::get_timestamp());

	local_entropy_player.record(folder + augs::get_timestamp() + "/" + filename);
}

bool entropy_player::is_replaying() const {
	return local_entropy_player.is_replaying();
}
