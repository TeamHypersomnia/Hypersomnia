#include <sstream>
#include <experimental/filesystem>

#include "desaturations.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

#include "augs/image/image.h"

namespace fs = std::experimental::filesystem;

void regenerate_desaturations() {
	const auto desaturations_directory = "generated/desaturations/";

	augs::create_directories(desaturations_directory);

	const auto lines = augs::get_file_lines("desaturations_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		desaturation_metadata new_meta;

		const auto source_path = fs::path(lines[current_line]);

		new_meta.last_write_time_of_source = augs::last_write_time(source_path.string());

		const auto desaturation_filename = desaturations_directory + source_path.filename().string();
		const auto desaturation_meta_filename = desaturations_directory + source_path.filename().replace_extension(".meta").string();

		augs::stream new_meta_stream;
		augs::write_object(new_meta_stream, new_meta);

		bool should_regenerate = false;

		if (!augs::file_exists(desaturation_filename)) {
			should_regenerate = true;
		}
		else {
			if (!augs::file_exists(desaturation_meta_filename)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_meta_stream;
				augs::assign_file_contents_binary(desaturation_meta_filename, existent_meta_stream);

				const bool are_metas_identical = (new_meta_stream == existent_meta_stream);

				if (!are_metas_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating desaturation for %x", source_path.string());

			augs::image source_image;
			source_image.from_file(source_path.string());
			source_image.get_desaturated().save(desaturation_filename);

			augs::create_binary_file(desaturation_meta_filename, new_meta_stream);
		}

		++current_line;
	}
}