#pragma once
#include <map>
#include "augs/filesystem/path.h"
#include "game/cosmos/cosmic_entropy.h"

class cosmic_movie_director {
public:
	unsigned player_step_position = 0u;

	std::map<unsigned, guid_mapped_entropy> step_to_entropy;

	guid_mapped_entropy get_entropy_for_step(const unsigned) const;

	bool is_recording_available() const;
	bool load_recording_from_file(const augs::path_type&);
	void save_recording_to_file(const augs::path_type&) const;
};