#include "cosmic_movie_director.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/templated_readwrite.h"

#include "augs/templates/string_templates.h"
#include <fstream>

void cosmic_movie_director::save_recording_to_file(const std::string& filename) const {
	//player.record(filename)
}

bool cosmic_movie_director::load_recording_from_file(const std::string& filename) {
	player_step_position = 0;
	step_to_entropy.clear();

	augs::read_map_until_eof(filename, step_to_entropy);

	return step_to_entropy.size() > 0;
}