#include "cosmic_movie_director.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/templated_readwrite.h"

#include "augs/templates/string_templates.h"

void cosmic_movie_director::load_recordings_from_directory(const std::string directory_path) {
	const auto files = augs::get_all_files_in_directory(directory_path);

	for (const auto& filename_without_directory : files) {
		std::istringstream f(filename_without_directory);
		std::string guid_string;
		std::getline(f, guid_string, '.');

		const auto guid = to_value<unsigned>(guid_string);

		guid_to_recording[guid].load_recording(directory_path + filename_without_directory);
	}
}