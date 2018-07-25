#include "game/cosmos/cosmic_movie_director.h"

#include "augs/filesystem/directory.h"
#include "augs/readwrite/byte_file.h"

void cosmic_movie_director::save_recording_to_file(const augs::path_type& path) const {
	auto f = augs::with_exceptions<std::ofstream>();
	f.open(path, std::ios::out | std::ios::binary);

	for (const auto& it : step_to_entropy) {
		augs::write_bytes(f, it.first);
		augs::write_bytes(f, it.second);
	}
}

guid_mapped_entropy cosmic_movie_director::get_entropy_for_step(const unsigned step) const {
	const auto it = step_to_entropy.find(step);

	if (it != step_to_entropy.end()) {
		return (*it).second;
	}

	return {};
}

bool cosmic_movie_director::is_recording_available() const {
	return step_to_entropy.size() > 0;
}

bool cosmic_movie_director::load_recording_from_file(const augs::path_type& path) {
	player_step_position = 0;
	step_to_entropy.clear();

	augs::read_map_until_eof(path, step_to_entropy);

	return is_recording_available();
}