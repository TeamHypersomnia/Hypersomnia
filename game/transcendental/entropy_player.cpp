#include <cereal/types/vector.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/archives/portable_binary.hpp>

#include "entropy_player.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"
#include "game/transcendental/cosmos.h"
#include "game/components/input_receiver_component.h"

void entropy_player::buffer_entropy_for_next_step(augs::machine_entropy delta) {
	total_buffered_entropy += delta;
}

augs::machine_entropy entropy_player::obtain_machine_entropy_for_next_step() {
	local_entropy_player.biserialize(total_buffered_entropy.local);
	
	auto resultant_entropy = total_buffered_entropy;
	total_buffered_entropy = augs::machine_entropy();

	return resultant_entropy;
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
