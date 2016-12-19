#pragma once
#include <unordered_map>
#include "cosmic_entropy.h"

class cosmic_movie_director {
public:
	unsigned player_step_position = 0u;

	std::unordered_map<unsigned, guid_mapped_entropy> step_to_entropy;

	bool load_recording_from_file(const std::string);
	void save_recording_to_file(const std::string) const;
};