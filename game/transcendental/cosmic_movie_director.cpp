#include "cosmic_movie_director.h"
#include "augs/filesystem/directory.h"

#include "augs/templates/string_templates.h"

void cosmic_movie_director::load_recordings_from_directory(const std::string directory_path) {
	const auto files = augs::get_all_files_in_directory(directory_path);

	for (const auto& f : files) {
		std::istringstream fname(f);
		std::string guid_string;
		std::getline(fname, guid_string, '.');

		const auto guid = to_value<unsigned>(guid_string);

		LOG("%x", guid);
	}
}