#include "cosmic_movie_director.h"
#include "augs/filesystem/directory.h"

void cosmic_movie_director::load_recordings_from_directory(const std::string directory_path) {
	const auto files = augs::get_all_files_in_directory(directory_path);

	for (const auto& f : files) {
		LOG(f);
	}
}