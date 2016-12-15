#pragma once
#include <map>

#include "augs/misc/step_player.h"
#include "cosmic_entropy.h"

class cosmic_movie_director {

public:
	std::map<unsigned, guid_mapped_entropy> step_to_entropy;
	
	augs::step_player<guid_mapped_entropy> player;

	void load_recording_from_file(const std::string);
	void save_recording_to_file(const std::string) const;
};