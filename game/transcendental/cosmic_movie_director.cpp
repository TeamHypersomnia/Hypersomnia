#include "cosmic_movie_director.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/templated_readwrite.h"

#include "augs/templates/string_templates.h"
#include <fstream>

void cosmic_movie_director::save_recording_to_file(const std::string filename) const {
	//player.record(filename)
}

bool cosmic_movie_director::load_recording_from_file(const std::string filename) {
	player_step_position = 0;
	std::ifstream source(filename, std::ios::in | std::ios::binary);

	while (source.peek() != EOF) {
		unsigned step;
		guid_mapped_entropy ent;

		augs::read_object(source, step);
		augs::read_object(source, ent);

		step_to_entropy.emplace(step, std::move(ent));
	}

	return step_to_entropy.size() > 0;
}