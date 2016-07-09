#include "entropy_player.h"
#include "filesystem/file.h"
#include "filesystem/directory.h"
#include "misc/time.h"
#include "game/cosmos.h"
#include "game/components/input_receiver_component.h"

void entropy_player::buffer_entropy_for_next_step(augs::machine_entropy delta) {
	total_buffered_entropy += delta;
}

cosmic_entropy entropy_player::obtain_cosmic_entropy_for_next_step(const cosmos& cosm) {
	local_entropy_player.biserialize(total_buffered_entropy.local);

	cosmic_entropy result;

	auto targets = cosm.get(processing_subjects::WITH_INPUT_RECEIVER);

	for (auto it : targets) {
		if (it.get<components::input_receiver>().local) {
			cosmic_entropy new_entropy;
			new_entropy.entropy_per_entity[it] = total_buffered_entropy.local;

			result += new_entropy;
		}
	}

	return result;
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
